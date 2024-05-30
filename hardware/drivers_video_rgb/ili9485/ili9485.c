/*
  ili9485.c

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

#include "ili9485.h"

#include <driver/gpio.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "esp3d_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief Initializes a new ILI9485 LCD panel.
 *
 * This function initializes a new ILI9485 LCD panel based on the provided
 * configuration.
 *
 * @param disp_panel_cfg Pointer to the configuration structure for the ILI9485
 * panel.
 * @param disp_panel Pointer to the handle of the initialized ILI9485 panel.
 *
 * @return `ESP_OK` if the panel is successfully initialized, or an error code
 * if initialization fails.
 */
esp_err_t esp_lcd_new_panel_ili9485(
    const esp_rgb_ili9485_config_t *disp_panel_cfg,
    esp_lcd_panel_handle_t *disp_panel) {
  esp_err_t ret = ESP_OK;
  if (esp_lcd_new_rgb_panel(&(disp_panel_cfg->panel_config), disp_panel) !=
      ESP_OK) {
    esp3d_log_e("Failed to create new RGB panel");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_reset(*disp_panel) != ESP_OK) {
    esp3d_log_e("Failed to reset panel");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_init(*disp_panel) != ESP_OK) {
    esp3d_log_e("Failed to initialize panel");
    return ESP_FAIL;
  }

  if (disp_panel_cfg->orientation == orientation_portrait ||
      disp_panel_cfg->orientation == orientation_portrait_invert) {
    if (esp_lcd_panel_swap_xy(*disp_panel, true) != ESP_OK) {
      esp3d_log_e("Failed to swap xy");
      return ESP_FAIL;
    }
  }

  if (disp_panel_cfg->orientation == orientation_landscape_invert ||
      disp_panel_cfg->orientation == orientation_portrait_invert) {
    if (esp_lcd_panel_mirror(*disp_panel, true, true) != ESP_OK) {
      esp3d_log_e("Failed to mirror");
      return ESP_FAIL;
    }
  }
  return ret;
}
