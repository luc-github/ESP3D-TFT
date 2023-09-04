/**
 * @file st7796.h
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

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "disp_def.h"


/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/



/**********************
 * GLOBAL PROTOTYPES
 **********************/

esp_err_t st7796_init();
esp_lcd_panel_handle_t * st7796_panel_handle();
void st7796_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif


