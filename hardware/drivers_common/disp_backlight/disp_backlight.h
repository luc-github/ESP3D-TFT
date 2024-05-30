/*
  disp_backlight.h

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

/*********************
 *      INCLUDES
 *********************/
#include <esp_err.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" { /* extern "C" */
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Structure representing the display backlight configuration.
 *
 * This structure holds the configuration parameters for controlling the display
 * backlight.
 */
typedef struct {
  bool pwm_control;  // true: LEDC is used, false: GPIO is used
  int index;         // Either GPIO or LEDC channel
} disp_backlight_t;

/**
 * @brief Configuration structure of backlight controller
 *
 * Must be passed to disp_backlight_create() for correct configuration
 */
typedef struct {
  bool pwm_control;    // true: LEDC is used, false: GPIO is used
  bool output_invert;  // true: LEDC output is inverted, false: LEDC output is
                       // not inverted
  int gpio_num;        // see gpio_num_t

  // Relevant only for PWM controlled backlight
  // Ignored for switch (ON/OFF) backlight control
  int timer_idx;    // ledc_timer_t
  int channel_idx;  // ledc_channel_t
} disp_backlight_config_t;

/**
 * @brief Creates a display backlight instance.
 *
 * This function creates a display backlight instance based on the provided
 * configuration.
 *
 * @param config Pointer to the backlight configuration.
 * @param bckl Pointer to the created backlight instance.
 * @return `ESP_OK` if the backlight instance is created successfully, or an
 * error code if it fails.
 */
esp_err_t disp_backlight_create(const disp_backlight_config_t *config,
                                disp_backlight_t **bckl);

/**
 * @brief Sets the brightness of the display backlight.
 *
 * This function sets the brightness of the display backlight to the specified
 * percentage.
 *
 * @param bckl Pointer to the `disp_backlight_t` structure representing the
 * display backlight.
 * @param brightness_percent The brightness level to set, specified as a
 * percentage (0-100).
 *
 * @return `ESP_OK` if the brightness was set successfully, or an error code if
 * an error occurred.
 */
esp_err_t disp_backlight_set(disp_backlight_t *bckl, int brightness_percent);

/**
 * @brief Deletes a disp_backlight_t object and frees the associated memory.
 *
 * This function deletes a disp_backlight_t object and frees the memory
 * allocated for it.
 *
 * @param bckl Pointer to the pointer of the disp_backlight_t object to be
 * deleted. After successful deletion, the pointer will be set to NULL.
 *
 * @return
 *     - ESP_OK if the disp_backlight_t object was successfully deleted.
 *     - ESP_ERR_INVALID_ARG if the pointer to the disp_backlight_t object is
 * NULL.
 */
esp_err_t disp_backlight_delete(disp_backlight_t **bckl);

#ifdef __cplusplus
} /* extern "C" */
#endif
