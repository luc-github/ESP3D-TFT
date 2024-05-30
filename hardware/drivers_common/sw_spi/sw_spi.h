/*
  sw_spi.h

  Copyright (c) 2023 serisman (https://github.com/serisman). All rights
  reserved.

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
#include <esp_err.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  int8_t cs_pin;
  int8_t clk_pin;
  int8_t mosi_pin;
  int8_t miso_pin;
} sw_spi_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the software SPI interface with the provided
 * configuration.
 *
 * This function initializes the software SPI interface using the provided
 * configuration.
 *
 * @param config Pointer to the configuration structure containing the SPI
 * settings.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t sw_spi_init(const sw_spi_config_t *config);

/**
 * @brief Reads a 16-bit register value using software SPI.
 *
 * This function reads a 16-bit register value using software SPI.
 *
 * @param reg The register to read.
 * @return The 16-bit value read from the register.
 */
uint16_t sw_spi_read_reg16(uint8_t reg);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
