/*
  esp32_camera.c

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

#include "esp32_camera.h"

#include <string.h>

#include "esp3d_log.h"

/**********************
 *  STATIC VARIABLES
 **********************/
int _camera_pin_led = -1;
char *_camera_name = NULL;

/*********************
 * GLOBAL PROTOTYPES
 * *********************/
/**
 * @brief Initializes the ESP32 camera module with the provided configuration.
 *
 * This function initializes the ESP32 camera module using the specified
 * configuration.
 *
 * @param config Pointer to the configuration structure containing camera
 * settings.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t esp32_camera_init(const esp32_camera_config_t *config) {
  if (config == NULL) {
    esp3d_log_e("Camera config is NULL");
    return ESP_ERR_INVALID_ARG;
  }
  // Set extra pullup if needed
  if (config->pin_pullup_1 != -1) {
    gpio_set_direction(config->pin_pullup_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(config->pin_pullup_1, GPIO_PULLUP_ONLY);
  }

  if (config->pin_pullup_2 != -1) {
    gpio_set_direction(config->pin_pullup_2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(config->pin_pullup_2, GPIO_PULLUP_ONLY);
  }

  // Set led pin
  if (config->pin_led != -1) {
    gpio_set_direction(config->pin_led, GPIO_MODE_OUTPUT);
    gpio_set_level(config->pin_led, 0);
    // Save camera pin led
    _camera_pin_led = config->pin_led;
  }
  // Save camera name
  _camera_name = (char *)calloc(strlen(config->name) + 1, sizeof(char));
  if (_camera_name == NULL) {
    esp3d_log_e("Camera name allocation failed");
    return ESP_ERR_NO_MEM;
  }
  _camera_name = strcpy(_camera_name, config->name);

  // Initilize camera
  esp3d_log("Camera init");
  esp_err_t err = esp_camera_init(&(config->hw_config));
  if (err != ESP_OK) {
    esp3d_log_e("Camera init failed with error 0x%x", err);
    return err;
  }

  // camera config
  sensor_t *s = esp_camera_sensor_get();
  if (!s) {
    esp3d_log_e("Camera sensor not found");
    return ESP_FAIL;
  }
  // Set camera parameters
  // framesize
  s->set_framesize(s, config->hw_config.frame_size);
  // brightness
  if (config->brightness != 0) {
    s->set_brightness(s, config->brightness);
  }
  // contrast
  if (config->contrast != 0) {
    s->set_contrast(s, config->contrast);
  }
  // flip H
  if (config->flip_horizontaly) {
    s->set_hmirror(s, 1);
  }
  // flip V
  if (config->flip_vertically) {
    s->set_vflip(s, 1);
  }

  return ESP_OK;
}

/**
 * @brief Controls the power LED of the ESP32 camera.
 *
 * This function turns the power LED of the ESP32 camera on or off based on the
 * provided parameter.
 *
 * @param on Boolean value indicating whether to turn the power LED on (true) or
 * off (false).
 * @return `ESP_OK` if the operation is successful, `ESP_ERR_INVALID_STATE` if
 * the camera pin LED is not configured.
 */
esp_err_t esp32_camera_power_led(bool on) {
  if (_camera_pin_led == -1) {
    return ESP_ERR_INVALID_STATE;
  }
  gpio_set_level(_camera_pin_led, on);
  return ESP_OK;
}

/**
 * @brief Get the name of the ESP32 camera.
 *
 * This function returns the name of the ESP32 camera as a null-terminated
 * string.
 *
 * @return The name of the ESP32 camera.
 */
const char *esp32_camera_get_name(void) { return (const char *)_camera_name; }