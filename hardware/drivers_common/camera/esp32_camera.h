/*
  esp32_camera.h

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
#include <esp_err.h>
#include <stdbool.h>

#include "esp_camera.h"

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Structure representing the configuration for the ESP32 camera.
 */
typedef struct {
  camera_config_t hw_config; /**< Hardware configuration for the camera. */
  int pin_pullup_1;          /**< Pin number for the first pull-up resistor. */
  int pin_pullup_2;          /**< Pin number for the second pull-up resistor. */
  int pin_led;               /**< Pin number for the LED. */
  bool flip_horizontaly;     /**< Flag indicating if the image should be flipped
                                horizontally. */
  bool flip_vertically;      /**< Flag indicating if the image should be flipped
                                vertically. */
  int brightness;            /**< Brightness level for the camera. */
  int contrast;              /**< Contrast level for the camera. */
  const char *name;          /**< Name of the camera. */
} esp32_camera_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the ESP32 camera module with the provided configuration.
 *
 * This function initializes the ESP32 camera module with the specified
 * configuration.
 *
 * @param config Pointer to the configuration structure containing camera
 * settings.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t esp32_camera_init(const esp32_camera_config_t *config);

/**
 * @brief Controls the power LED of the ESP32 camera.
 *
 * This function is used to turn the power LED of the ESP32 camera on or off.
 *
 * @param on Boolean value indicating whether to turn the power LED on (true) or
 * off (false).
 * @return `ESP_OK` if the operation is successful, otherwise an error code
 * indicating the cause of the failure.
 */
esp_err_t esp32_camera_power_led(bool on);

/**
 * @brief Get the name of the ESP32 camera.
 *
 * This function returns a pointer to a constant character string representing
 * the name of the ESP32 camera.
 *
 * @return Pointer to a constant character string representing the name of the
 * ESP32 camera.
 */
const char *esp32_camera_get_name(void);

#ifdef __cplusplus
}
#endif
