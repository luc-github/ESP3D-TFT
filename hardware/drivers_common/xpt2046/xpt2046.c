/*
  xtp2046.c

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
#include "xpt2046.h"

#include <driver/gpio.h>
#include <stddef.h>

#include "esp3d_log.h"
#include "esp_system.h"

/*********************
 *      DEFINES
 *********************/
// XPT2046 commands
#define CMD_X_READ 0b11010000
#define CMD_Y_READ 0b10010000
#define CMD_Z1_READ 0b10110000
#define CMD_Z2_READ 0b11000000

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
  TOUCH_NOT_DETECTED = 0,
  TOUCH_DETECTED = 1,
} xpt2046_touch_detect_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static xpt2046_touch_detect_t xpt2048_is_touch_detected();

/**********************
 *  STATIC VARIABLES
 **********************/
static const xpt2046_config_t *xpt2046_config = NULL;
static uint16_t _xtp2046_x_max = 0;
static uint16_t _xtp2046_y_max = 0;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the XPT2046 touch controller.
 *
 * This function initializes the XPT2046 touch controller with the provided
 * configuration.
 *
 * @param config Pointer to the configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t xpt2046_init(const xpt2046_config_t *config) {
  if (config == NULL || config->read_reg12_fn == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  _xtp2046_x_max = config->x_max;
  _xtp2046_y_max = config->y_max;
  xpt2046_config = config;
  esp_err_t err = ESP_OK;
  esp3d_log("Init XPT2046 driver");
  if (GPIO_IS_VALID_GPIO(config->irq_pin)) {
    gpio_config_t irq_config = {
        .pin_bit_mask = BIT64(config->irq_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&irq_config);

  } else {
    err = ESP_ERR_INVALID_ARG;
  }
  if (err != ESP_OK) {
    esp3d_log("Error configuring IRQ pin: %d", config->irq_pin);
  }
  return err;
}

/**
 * Reads data from the XPT2046 touch controller.
 *
 * @return The data read from the XPT2046 touch controller.
 */
xpt2046_data_t xpt2046_read() {
  xpt2046_data_t data = {.is_pressed = false, .x = -1, .y = -1};
  if (xpt2046_config == NULL) {
    esp3d_log_e("XPT2046 not initialized");
    return data;
  }
  if (xpt2048_is_touch_detected() == TOUCH_DETECTED) {
    data.is_pressed = true;
    data.x = xpt2046_config->read_reg12_fn(CMD_X_READ);
    data.y = xpt2046_config->read_reg12_fn(CMD_Y_READ);
    if (xpt2046_config->swap_xy) {
      int16_t swap_tmp = data.x;
      data.x = data.y;
      data.y = swap_tmp;
    }
    if (xpt2046_config->invert_x) {
      data.x = 4095 - data.x;
    }
    if (xpt2046_config->invert_y) {
      data.y = 4095 - data.y;
    }
    esp3d_log("P(%d,%d)", data.x, data.y);
  }
  return data;
}

/**
 * @brief Retrieves the maximum x-coordinate value for the XPT2046 touch
 * controller.
 *
 * @return The maximum x-coordinate value.
 */
uint16_t get_xtp2046_x_max() { return _xtp2046_x_max; }

/**
 * @brief Retrieves the maximum Y coordinate value for the XPT2046 touch
 * controller.
 *
 * @return The maximum Y coordinate value.
 */
uint16_t get_xtp2046_y_max() { return _xtp2046_y_max; }

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Checks if touch is detected by the XPT2046 touch controller.
 *
 * @return The touch detection status:
 *         - `XPT2046_TOUCH_DETECTED` if touch is detected.
 *         - `XPT2046_TOUCH_NOT_DETECTED` if touch is not detected.
 */
static xpt2046_touch_detect_t xpt2048_is_touch_detected() {
  if (xpt2046_config == NULL) {
    esp3d_log_e("XPT2046 not initialized");
    return TOUCH_NOT_DETECTED;
  }

  // Check if IRQ pin is triggered
  if (xpt2046_config->irq_pin >= 0) {
    uint8_t irq = gpio_get_level(xpt2046_config->irq_pin);
    if (irq != 0) {
      return TOUCH_NOT_DETECTED;
    }
  }

  // Check touch pressure
  int16_t z1 = xpt2046_config->read_reg12_fn(CMD_Z1_READ);
  int16_t z2 = xpt2046_config->read_reg12_fn(CMD_Z2_READ);

  // This is not what the confusing datasheet says but it seems to
  //   be enough to detect real touches on the panel.
  int16_t z = z1 + 4095 - z2;
  if (z < xpt2046_config->touch_threshold) {
    return TOUCH_NOT_DETECTED;
  }

  return TOUCH_DETECTED;
}
