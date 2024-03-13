/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "esp3d_log.h"
#include "esp_check.h"
#include "st7262.h"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

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


/**********************
 *   Static FUNCTIONS
 **********************/
