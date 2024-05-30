/*
  xtp2046.h

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

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
// Function to read 12 bits from the XPT2046
typedef uint16_t (*xpt2046_read_reg12_fn_t)(uint8_t reg);

// Configuration structure for XPT2046
typedef struct {
  int8_t irq_pin;
  xpt2046_read_reg12_fn_t read_reg12_fn;
  uint16_t touch_threshold;
  bool swap_xy;
  bool invert_x;
  bool invert_y;
  uint16_t x_max;
  uint16_t y_max;
  uint16_t calibration_x_min;
  uint16_t calibration_y_min;
  uint16_t calibration_x_max;
  uint16_t calibration_y_max;
} xpt2046_config_t;

// Data structure for touch position and state
typedef struct {
  int16_t x;
  int16_t y;
  bool is_pressed;
} xpt2046_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the XPT2046 touch controller.
 *
 * This function initializes the XPT2046 touch controller with the provided
 * configuration.
 *
 * @param config Pointer to the XPT2046 configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t xpt2046_init(const xpt2046_config_t *config);

/**
 * Reads data from the XPT2046 touch controller.
 *
 * @return The data read from the XPT2046 touch controller.
 */
xpt2046_data_t xpt2046_read();

/**
 * @brief Retrieves the maximum x-coordinate value supported by the XPT2046
 * touch controller.
 *
 * @return The maximum x-coordinate value.
 */
uint16_t get_xtp2046_x_max();

/**
 * @brief Retrieves the maximum Y coordinate value supported by the XPT2046
 * touch controller.
 *
 * @return The maximum Y coordinate value.
 */
uint16_t get_xtp2046_y_max();

#ifdef __cplusplus
} /* extern "C" */
#endif
