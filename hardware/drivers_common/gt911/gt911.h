/**
 * @file gt911.h
 * @brief gt911 driver header file.
 * @version 0.1
 * @date 2021-03-07
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "esp_err.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      TYPEDEFS
 **********************/
// Configuration structure for GT911
typedef struct {
  uint32_t i2c_clk_speed;
  uint8_t *i2c_addr;
  int8_t rst_pin;
  int8_t int_pin;
  bool swap_xy;
  bool invert_x;
  bool invert_y;
  uint16_t x_max;
  uint16_t y_max;
} gt911_config_t;

// Data structure for touch position and state
typedef struct {
  bool is_pressed;
  int16_t x;
  int16_t y;
} gt911_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the gt911 touch controller.
 *
 * This function initializes the gt911 touch controller with the provided
 * configuration and i2c handle.
 *
 * @param i2c_bus i2c bus handle used by the driver.
 * @param config Pointer to the configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t gt911_init(i2c_bus_handle_t i2c_bus, const gt911_config_t *config);

/**
 * Reads data from the GT911 touch controller.
 *
 * @return The data read from the GT911 touch controller.
 */
gt911_data_t gt911_read();

/**
 * @brief Get the maximum x-coordinate value supported by the GT911 touch
 * controller.
 *
 * @return The maximum x-coordinate value.
 */
uint16_t get_gt911_x_max();

/**
 * @brief Retrieves the maximum Y coordinate value supported by the GT911 touch
 * controller.
 *
 * @return The maximum Y coordinate value.
 */
uint16_t get_gt911_y_max();

#ifdef __cplusplus
}
#endif
