/*
  st7262.c

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

#include <stdlib.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "esp3d_log.h"
#include "esp_check.h"
#include "st7262.h"


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
esp_err_t esp_lcd_new_panel_st7262(const esp_lcd_rgb_panel_config_t *disp_panel_cfg, esp_lcd_panel_handle_t *disp_panel){
  esp_err_t ret = ESP_OK;
  if (esp_lcd_new_rgb_panel(disp_panel_cfg, disp_panel)!=ESP_OK){
    esp3d_log_e( "Failed to create new RGB panel");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_reset(*disp_panel)!=ESP_OK){
        esp3d_log_e("Failed to reset panel");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_init(*disp_panel)!=ESP_OK){
        esp3d_log_e( "Failed to initialize panel");
    return ESP_FAIL;
  }
  return ret;
}
