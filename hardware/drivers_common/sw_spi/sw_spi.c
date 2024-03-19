/*
  sw_spi.c

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

/*********************
 *      INCLUDES
 *********************/
#include "sw_spi.h"

#include <driver/gpio.h>

#include "esp3d_log.h"
#include "esp_system.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static esp_err_t sw_spi_acquire_bus();
static void sw_spi_release_bus();
static esp_err_t sw_spi_write_byte(uint8_t data);
static uint8_t sw_spi_read_byte();

/**********************
 *  STATIC VARIABLES
 **********************/
static const sw_spi_config_t *_config = NULL;
static bool _spi_bus_acquired = false;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the software SPI interface.
 *
 * This function initializes the software SPI interface with the provided
 * configuration.
 *
 * @param config Pointer to the software SPI configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t sw_spi_init(const sw_spi_config_t *config) {
  if (config == NULL) {
    esp3d_log_e("Invalid config");
    return ESP_ERR_INVALID_ARG;
  }
  _config = config;
  esp_err_t err = ESP_OK;
  esp3d_log("Initializing software SPI pins...");
  esp3d_log("MISO pin: %d, MOSI pin: %d, SCLK pin: %d, CS pin: %d",
            config->miso_pin, config->mosi_pin, config->clk_pin,
            config->cs_pin);
  // Configure the GPIO pins
  // CS pin
  if (GPIO_IS_VALID_GPIO(config->cs_pin)) {
    esp_rom_gpio_pad_select_gpio(config->cs_pin);
    gpio_set_direction(config->cs_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(config->cs_pin, 1);
    _spi_bus_acquired = false;
  } else {
    esp3d_log_e("CS pin not configured");
    return ESP_ERR_INVALID_ARG;
  }

  // CLK pin
  if (GPIO_IS_VALID_GPIO(config->clk_pin)) {
    esp_rom_gpio_pad_select_gpio(config->clk_pin);
    gpio_set_direction(config->clk_pin, GPIO_MODE_OUTPUT);
  } else {
    esp3d_log_e("CLK pin not configured");
    return ESP_ERR_INVALID_ARG;
  }
  // MOSI pin
  if (GPIO_IS_VALID_GPIO(config->mosi_pin)) {
    esp_rom_gpio_pad_select_gpio(config->mosi_pin);
    gpio_set_direction(config->mosi_pin, GPIO_MODE_OUTPUT);

    // MISO pin
    gpio_config_t miso_config = {
        .pin_bit_mask = BIT64(config->miso_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    err = gpio_config(&miso_config);
    if (err != ESP_OK) {
      esp3d_log_e("Failed to configure SW SPI  MISO pin: %d", err);
    } else {
      esp3d_log("SW SPI MISO pin configured successfully");
    }
  } else {
    esp3d_log_e("MISO pin not configured");
    return ESP_ERR_INVALID_ARG;
  }
  gpio_set_level(config->cs_pin, 1);
  return err;
}

/**
 * Reads a 16-bit register from the software SPI.
 *
 * @param reg The register to read.
 * @return The value read from the register.
 */
uint16_t sw_spi_read_reg16(uint8_t reg) {
  if (sw_spi_acquire_bus() != ESP_OK) {
    esp3d_log_e("Failed to acquire SPI bus");
    return 0;
  }
  // Send the register to read
  sw_spi_write_byte(reg);
  // Read the 16-bit value
  uint8_t d0 = sw_spi_read_byte();
  uint8_t d1 = sw_spi_read_byte();
  // Release the SPI bus
  sw_spi_release_bus();
  // Return the 16-bit value
  return ((uint16_t)d0 << 8) | d1;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Acquires the software SPI bus.
 *
 * This function is responsible for acquiring the software SPI bus before
 * performing any SPI transactions. It ensures exclusive access to the SPI bus
 * to prevent conflicts between multiple devices.
 */
static esp_err_t sw_spi_acquire_bus() {
  // Need to check if the bus is already acquired
  if (_spi_bus_acquired) {
    esp3d_log_e("SPI bus already acquired");
    return ESP_FAIL;
  }
  gpio_set_level(_config->clk_pin, 0);
  gpio_set_level(_config->mosi_pin, 0);
  gpio_set_level(_config->cs_pin, 0);
  _spi_bus_acquired = true;
  return ESP_OK;
}

/**
 * @brief Releases the software SPI bus by setting the CS pin to high.
 *
 * This function is used to release the software SPI bus by setting the CS pin
 * to high. After calling this function, the bus is available for other devices
 * to use.
 */
static void sw_spi_release_bus() {
  gpio_set_level(_config->cs_pin, 1);
  _spi_bus_acquired = false;
}

/**
 * @brief Writes a byte of data using software SPI.
 *
 * This function writes a byte of data using software SPI.
 *
 * @param data The byte of data to be written.
 */
static esp_err_t sw_spi_write_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    // set MOSI
    gpio_set_level(_config->mosi_pin, (data & 0x80) ? 1 : 0);
    gpio_set_level(_config->clk_pin, 1);
    data <<= 1;
    // (read MISO)
    // if (gpio_get_level(_config->miso_pin)) {
    //   data++;
    // }
    gpio_set_level(_config->clk_pin, 0);
  }
  return ESP_OK;
}

/**
 * @brief Reads a byte using software SPI.
 *
 * This function reads a byte using software SPI protocol.
 *
 * @return The byte read from the SPI bus.
 */
static uint8_t sw_spi_read_byte() {
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++) {
    // (set MOSI)
    // gpio_set_level(_config->mosi_pin, (data & 0x80) ? 1 : 0);
    gpio_set_level(_config->clk_pin, 1);
    data <<= 1;
    // read MISO
    if (gpio_get_level(_config->miso_pin)) {
      data++;
    }
    gpio_set_level(_config->clk_pin, 0);
  }
  return data;
}
