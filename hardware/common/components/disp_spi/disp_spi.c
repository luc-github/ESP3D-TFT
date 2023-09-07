/**
 * @file disp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "esp_system.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include "esp3d_log.h"
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "disp_spi.h"

/******************************************************************************
 * Notes about DMA spi_transaction_ext_t structure pooling
 * 
 * An xQueue is used to hold a pool of reusable SPI spi_transaction_ext_t 
 * structures that get used for all DMA SPI transactions. While an xQueue may 
 * seem like overkill it is an already built-in RTOS feature that comes at 
 * little cost. xQueues are also ISR safe if it ever becomes necessary to 
 * access the pool in the ISR callback.
 * 
 * When a DMA request is sent, a transaction structure is removed from the 
 * pool, filled out, and passed off to the esp32 SPI driver. Later, when 
 * servicing pending SPI transaction results, the transaction structure is 
 * recycled back into the pool for later reuse. This matches the DMA SPI 
 * transaction life cycle requirements of the esp32 SPI driver.
 * 
 * When polling or synchronously sending SPI requests, and as required by the 
 * esp32 SPI driver, all pending DMA transactions are first serviced. Then the 
 * polling SPI request takes place. 
 * 
 * When sending an asynchronous DMA SPI request, if the pool is empty, some 
 * small percentage of pending transactions are first serviced before sending 
 * any new DMA SPI transactions. Not too many and not too few as this balance 
 * controls DMA transaction latency.
 * 
 * It is therefore not the design that all pending transactions must be 
 * serviced and placed back into the pool with DMA SPI requests - that 
 * will happen eventually. The pool just needs to contain enough to float some 
 * number of in-flight SPI requests to speed up the overall DMA SPI data rate 
 * and reduce transaction latency. If however a display driver uses some 
 * polling SPI requests or calls disp_wait_for_pending_transactions() directly,
 * the pool will reach the full state more often and speed up DMA queuing.
 * 
 *****************************************************************************/

/*********************
 *      DEFINES
 *********************/
#define SPI_TRANSACTION_POOL_SIZE 50	/* maximum number of DMA transactions simultaneously in-flight */

/* DMA Transactions to reserve before queueing additional DMA transactions. A 1/10th seems to be a good balance. Too many (or all) and it will increase latency. */
#define SPI_TRANSACTION_POOL_RESERVE_PERCENTAGE 10
#if SPI_TRANSACTION_POOL_SIZE >= SPI_TRANSACTION_POOL_RESERVE_PERCENTAGE
#define SPI_TRANSACTION_POOL_RESERVE (SPI_TRANSACTION_POOL_SIZE / SPI_TRANSACTION_POOL_RESERVE_PERCENTAGE)	
#else
#define SPI_TRANSACTION_POOL_RESERVE 1	/* defines minimum size */
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void IRAM_ATTR spi_ready (spi_transaction_t *trans);
static void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg);

/**********************
 *  STATIC VARIABLES
 **********************/
static spi_host_device_t spi_host;
static spi_device_handle_t spi;
static QueueHandle_t TransactionPool = NULL;
static disp_spi_flush_cb_t _flush_cb = NULL;
static transaction_cb_t chained_post_cb = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void disp_spi_add_device(spi_host_device_t host, spi_device_interface_config_t * spi_dev_cfg, disp_spi_flush_cb_t flush_cb) {
    esp3d_log("Adding SPI device");
    esp3d_log("Clock speed: %dHz, mode: %d, CS pin: %d",
        spi_dev_cfg->clock_speed_hz, spi_dev_cfg->mode, spi_dev_cfg->spics_io_num);

    _flush_cb = flush_cb;
    spi_dev_cfg->queue_size = SPI_TRANSACTION_POOL_SIZE;
    disp_spi_add_device_config(host, spi_dev_cfg);

	/* create the transaction pool and fill it with ptrs to spi_transaction_ext_t to reuse */
	if(TransactionPool == NULL) {
		TransactionPool = xQueueCreate(SPI_TRANSACTION_POOL_SIZE, sizeof(spi_transaction_ext_t*));
		assert(TransactionPool != NULL);
		for (size_t i = 0; i < SPI_TRANSACTION_POOL_SIZE; i++)
		{
			spi_transaction_ext_t* pTransaction = (spi_transaction_ext_t*)heap_caps_malloc(sizeof(spi_transaction_ext_t), MALLOC_CAP_DMA);
			assert(pTransaction != NULL);
			memset(pTransaction, 0, sizeof(spi_transaction_ext_t));
			xQueueSend(TransactionPool, &pTransaction, portMAX_DELAY);
		}
	}
}

void disp_spi_remove_device() {
    /* Wait for previous pending transaction results */
    disp_wait_for_pending_transactions();

    esp_err_t ret = spi_bus_remove_device(spi);
    assert(ret==ESP_OK);
}

void disp_spi_transaction(const uint8_t *data, size_t length,
    disp_spi_send_flag_t flags, uint8_t *out,
    uint64_t addr, uint8_t dummy_bits)
{
    if (length == 0) {
        return;
    }

    spi_transaction_ext_t t = {0};

    /* transaction length is in bits */
    t.base.length = length * 8;

    if (length <= 4 && data != NULL) {
        t.base.flags = SPI_TRANS_USE_TXDATA;
        memcpy(t.base.tx_data, data, length);
    } else {
        t.base.tx_buffer = data;
    }

    if (flags & DISP_SPI_RECEIVE) {
        assert(out != NULL && (flags & (DISP_SPI_SEND_POLLING | DISP_SPI_SEND_SYNCHRONOUS)));
        t.base.rx_buffer = out;

// #if defined(DISP_SPI_HALF_DUPLEX)
// 		t.base.rxlength = t.base.length;
// 		t.base.length = 0;	/* no MOSI phase in half-duplex reads */
// #else
		t.base.rxlength = 0; /* in full-duplex mode, zero means same as tx length */
//#endif
    }

    if (flags & DISP_SPI_ADDRESS_8) {
        t.address_bits = 8;
    } else if (flags & DISP_SPI_ADDRESS_16) {
        t.address_bits = 16;
    } else if (flags & DISP_SPI_ADDRESS_24) {
        t.address_bits = 24;
    } else if (flags & DISP_SPI_ADDRESS_32) {
        t.address_bits = 32;
    }
    if (t.address_bits) {
        t.base.addr = addr;
        t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
    }

// #if defined(DISP_SPI_HALF_DUPLEX)
// 	if (flags & DISP_SPI_MODE_DIO) {
// 		t.base.flags |= SPI_TRANS_MODE_DIO;
// 	} else if (flags & DISP_SPI_MODE_QIO) {
// 		t.base.flags |= SPI_TRANS_MODE_QIO;
// 	}

// 	if (flags & DISP_SPI_MODE_DIOQIO_ADDR) {
// 		t.base.flags |= SPI_TRANS_MODE_DIOQIO_ADDR;
// 	}

// 	if ((flags & DISP_SPI_VARIABLE_DUMMY) && dummy_bits) {
// 		t.dummy_bits = dummy_bits;
// 		t.base.flags |= SPI_TRANS_VARIABLE_DUMMY;
// 	}
// #endif

    /* Save flags for pre/post transaction processing */
    t.base.user = (void *) flags;

    /* Poll/Complete/Queue transaction */
    if (flags & DISP_SPI_SEND_POLLING) {
		disp_wait_for_pending_transactions();	/* before polling, all previous pending transactions need to be serviced */
        spi_device_polling_transmit(spi, (spi_transaction_t *) &t);
    } else if (flags & DISP_SPI_SEND_SYNCHRONOUS) {
		disp_wait_for_pending_transactions();	/* before synchronous queueing, all previous pending transactions need to be serviced */
        spi_device_transmit(spi, (spi_transaction_t *) &t);
    } else {
		
		/* if necessary, ensure we can queue new transactions by servicing some previous transactions */
		if (uxQueueMessagesWaiting(TransactionPool) == 0) {
			spi_transaction_t *presult;
			while(uxQueueMessagesWaiting(TransactionPool) < SPI_TRANSACTION_POOL_RESERVE) {
				if (spi_device_get_trans_result(spi, &presult, 1) == ESP_OK) {
					xQueueSend(TransactionPool, &presult, portMAX_DELAY);	/* back to the pool to be reused */
				}
			}
		}

		spi_transaction_ext_t *pTransaction = NULL;
		xQueueReceive(TransactionPool, &pTransaction, portMAX_DELAY);
        memcpy(pTransaction, &t, sizeof(t));
        if (spi_device_queue_trans(spi, (spi_transaction_t *) pTransaction, portMAX_DELAY) != ESP_OK) {
			xQueueSend(TransactionPool, &pTransaction, portMAX_DELAY);	/* send failed transaction back to the pool to be reused */
        }
    }
}

void disp_wait_for_pending_transactions(void) {
    spi_transaction_t *presult;

	while (uxQueueMessagesWaiting(TransactionPool) < SPI_TRANSACTION_POOL_SIZE) {	/* service until the transaction reuse pool is full again */
        if (spi_device_get_trans_result(spi, &presult, 1) == ESP_OK) {
			xQueueSend(TransactionPool, &presult, portMAX_DELAY);
        }
    }
}

void disp_spi_acquire(void) {
    esp_err_t ret = spi_device_acquire_bus(spi, portMAX_DELAY);
    assert(ret == ESP_OK);
}

void disp_spi_release(void) {
    spi_device_release_bus(spi);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg) {
    spi_host = host;
    chained_post_cb = devcfg->post_cb;
    devcfg->post_cb = spi_ready;
    esp_err_t ret = spi_bus_add_device(host, devcfg, &spi);
    assert(ret==ESP_OK);
}

void spi_ready(spi_transaction_t *trans) {

    disp_spi_send_flag_t flags = (disp_spi_send_flag_t) trans->user;
    if ((flags & DISP_SPI_SIGNAL_FLUSH) && _flush_cb != NULL) {
        _flush_cb();
    }

    if (chained_post_cb) {
        chained_post_cb(trans);
    }
}

