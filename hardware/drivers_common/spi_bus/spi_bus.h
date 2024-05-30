/*
  spi_bus.h

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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <driver/spi_master.h>
#include <esp_err.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the SPI bus with the specified parameters.
 *
 * This function initializes the SPI bus with the given host, MISO, MOSI, and
 * SCLK pins, maximum transfer size, DMA channel, quadwp pin, and quadhd pin.
 *
 * @param host The SPI host number.
 * @param miso_pin The MISO pin number.
 * @param mosi_pin The MOSI pin number.
 * @param sclk_pin The SCLK pin number.
 * @param max_transfer_sz The maximum transfer size in bytes.
 * @param dma_channel The DMA channel number.
 * @param quadwp_pin The quadwp pin number.
 * @param quadhd_pin The quadhd pin number.
 * @return `ESP_OK` if the SPI bus is successfully initialized, otherwise an
 * error code.
 */
esp_err_t spi_bus_init(int host, int miso_pin, int mosi_pin, int sclk_pin,
                       int max_transfer_sz, int dma_channel, int quadwp_pin,
                       int quadhd_pin);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
