/**
 * @file spi_bus.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "spi_bus.h"
#include "esp3d_log.h"
#include <stdint.h>
#include <driver/spi_master.h>

/*********************
 *      DEFINES
 *********************/
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
void spi_bus_init(int host,
                  int miso_pin, int mosi_pin, int sclk_pin,
                  int max_transfer_sz,
                  int dma_channel,
                  int quadwp_pin, int quadhd_pin) {
    assert((0 <= host) && (SPI_HOST_MAX > host));
#if ESP3D_TFT_LOG
    const char *spi_names[] = {
        "SPI1_HOST", "SPI2_HOST", "SPI3_HOST"
    };
    (void)spi_names;
#endif //ESP3D_TFT_LOG
    esp3d_log("Configuring SPI host %s", spi_names[host]);
    esp3d_log("MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d",
        miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);
    esp3d_log("Max transfer size: %d (bytes)", max_transfer_sz);

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
        .mosi_io_num = mosi_pin,
        .sclk_io_num = sclk_pin,
        .quadwp_io_num = quadwp_pin,
        .quadhd_io_num = quadhd_pin,
        .max_transfer_sz = max_transfer_sz
    };

    esp3d_log("Initializing SPI bus...");
    esp_err_t ret = spi_bus_initialize(host, &buscfg, (spi_dma_chan_t)dma_channel);
    assert(ret == ESP_OK);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
