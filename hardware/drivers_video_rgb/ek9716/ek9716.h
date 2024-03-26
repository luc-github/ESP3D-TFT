/*
  ek9716.h

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
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_rgb.h"

/**********************
 *      TYPEDEFS
 **********************/
/**
 * @brief Enumeration for the orientation of the display.
 *
 * This enumeration defines the possible orientations of the display.
 * - `orientation_portrait`: The display is in portrait mode.
 * - `orientation_landscape`: The display is in landscape mode.
 * - `orientation_portrait_invert`: The display is in inverted portrait mode.
 * - `orientation_landscape_invert`: The display is in inverted landscape mode.
 */
typedef enum {
    orientation_portrait = 0,
    orientation_landscape = 1,
    orientation_portrait_invert = 2,
    orientation_landscape_invert = 3,
} esp_i80_ek9716_orientation_t;

/**
 * @brief Structure representing the configuration for the ek9716 RGB panel.
 */
typedef struct {
  esp_lcd_rgb_panel_config_t panel_config; /**< Configuration for the RGB panel. */
  esp_i80_ek9716_orientation_t orientation; /**< Orientation of the panel. */
  uint16_t hor_res; /**< Horizontal resolution of the panel. */
  uint16_t ver_res; /**< Vertical resolution of the panel. */
} esp_i80_ek9716_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes a new ek9716 LCD panel.
 *
 * This function initializes a new ek9716 LCD panel with the provided configuration.
 *
 * @param disp_panel_cfg Pointer to the configuration structure for the ek9716 panel.
 * @param disp_panel Pointer to the handle of the initialized ek9716 panel.
 *
 * @return `ESP_OK` if the panel is successfully initialized, or an error code if initialization fails.
 */
esp_err_t esp_lcd_new_panel_ek9716(const esp_i80_ek9716_config_t *disp_panel_cfg, esp_lcd_panel_handle_t *disp_panel);

#ifdef __cplusplus
} /* extern "C" */
#endif
