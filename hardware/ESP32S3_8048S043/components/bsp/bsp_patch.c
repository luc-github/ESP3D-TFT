/*
  esp3d_tft project

  Copyright (c) 2022 Luc Lebosse. All rights reserved.

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
#include "bsp_patch.h"

#if ESP3D_DISPLAY_FEATURE
#include "disp_def.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7262.h"

#endif  // ESP3D_DISPLAY_FEATURE

/*

*/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
esp_err_t bsp_accessFs(void) {
#if ESP3D_DISPLAY_FEATURE
  esp_lcd_panel_handle_t* h = get_st7262_panel_handle();
  esp_err_t ret = esp_lcd_rgb_panel_set_pclk(*h, 6 * 1000 * 1000);
  vTaskDelay(pdMS_TO_TICKS(40));
  return ret;
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}
esp_err_t bsp_releaseFs(void) {
#if ESP3D_DISPLAY_FEATURE
  esp_lcd_panel_handle_t* h = get_st7262_panel_handle();
  esp_err_t ret = esp_lcd_rgb_panel_set_pclk(*h, DISP_CLK_FREQ);
  vTaskDelay(pdMS_TO_TICKS(40));
  return ret;
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}