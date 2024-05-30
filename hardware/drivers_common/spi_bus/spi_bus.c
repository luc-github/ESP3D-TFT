/*
  spi_bus.c

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*********************
 *      INCLUDES
 *********************/

#include "spi_bus.h"

#include <driver/spi_master.h>
#include <stdint.h>

#include "esp3d_log.h"

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

/**
 * @brief Initialize the SPI bus master.
 *
 * This function initializes the SPI bus master with the specified parameters.
 *
 * @param host The SPI host number.
 * @param miso_pin The GPIO pin number for MISO (Master In Slave Out).
 * @param mosi_pin The GPIO pin number for MOSI (Master Out Slave In).
 * @param sclk_pin The GPIO pin number for the clock signal.
 * @param max_transfer_sz The maximum transfer size in bytes.
 * @param dma_channel The DMA channel to be used for SPI transfers.
 * @param quadwp_pin The GPIO pin number for the quad write protect signal.
 * @param quadhd_pin The GPIO pin number for the quad hold signal.
 *
 * @return `ESP_OK` on success, or an error code if initialization fails.
 *
 * NOTE: dma_chan type and value changed to int instead of spi_dma_chan_t
 * for backwards compatibility with ESP-IDF versions prior v4.3.
 *
 * We could use the ESP_IDF_VERSION_VAL macro available in the
 * "esp_idf_version.h" header available since ESP-IDF v4.
 */
esp_err_t spi_bus_init(int host, int miso_pin, int mosi_pin, int sclk_pin,
                       int max_transfer_sz, int dma_channel, int quadwp_pin,
                       int quadhd_pin) {
  if (host < 0 || host >= SPI_HOST_MAX) {
    esp3d_log("Invalid SPI host number: %d", host);
    return ESP_ERR_INVALID_ARG;
  }

#if ESP3D_TFT_LOG
  const char *spi_names[] = {"SPI1_HOST", "SPI2_HOST", "SPI3_HOST"};
  (void)spi_names;
#endif  // ESP3D_TFT_LOG
  esp3d_log("Configuring SPI host %s", spi_names[host]);
  esp3d_log(
      "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: "
      "%d",
      miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);
  esp3d_log("Max transfer size: %d (bytes)", max_transfer_sz);

  spi_bus_config_t buscfg = {.miso_io_num = miso_pin,
                             .mosi_io_num = mosi_pin,
                             .sclk_io_num = sclk_pin,
                             .quadwp_io_num = quadwp_pin,
                             .quadhd_io_num = quadhd_pin,
                             .max_transfer_sz = max_transfer_sz};

  esp3d_log("Initializing SPI bus...");
  esp_err_t ret =
      spi_bus_initialize(host, &buscfg, (spi_dma_chan_t)dma_channel);

  if (ret != ESP_OK) {
    esp3d_log_e("Failed to initialize SPI bus");
  } else {
    esp3d_log("SPI bus initialized successfully");
  }
  return ret;
}
