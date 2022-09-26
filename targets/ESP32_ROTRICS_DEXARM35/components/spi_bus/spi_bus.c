/**
 * @file spi_bus.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "esp_log.h"
#include <stdint.h>
#include <driver/spi_master.h>
#include "spi_bus.h"

/*********************
 *      DEFINES
 *********************/
#define TAG "SPI_BUS"
#define SPI_HOST_MAX 3

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
 
 /* Initialize spi bus master
 *
 * NOTE: dma_chan type and value changed to int instead of spi_dma_chan_t
 * for backwards compatibility with ESP-IDF versions prior v4.3.
 *
 * We could use the ESP_IDF_VERSION_VAL macro available in the "esp_idf_version.h"
 * header available since ESP-IDF v4.
 */
bool spi_driver_init(int host,
    int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz,
    int dma_channel,
    int quadwp_pin, int quadhd_pin){
    assert((0 <= host) && (SPI_HOST_MAX > host));
    const char *spi_names[] = {
        "SPI1_HOST", "SPI2_HOST", "SPI3_HOST"
    };

    ESP_LOGI(TAG, "Configuring SPI host %s", spi_names[host]);
    ESP_LOGI(TAG, "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d",
        miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);

    ESP_LOGI(TAG, "Max transfer size: %d (bytes)", max_transfer_sz);

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = quadwp_pin,
        .quadhd_io_num = quadhd_pin,
        .max_transfer_sz = max_transfer_sz
    };

    ESP_LOGI(TAG, "Initializing SPI bus...");
    #if defined (CONFIG_IDF_TARGET_ESP32C3)
    dma_channel = SPI_DMA_CH_AUTO;
    #endif
    esp_err_t ret = spi_bus_initialize(host, &buscfg, (spi_dma_chan_t)dma_channel);
    assert(ret == ESP_OK);

    return ESP_OK != ret;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
 
 

