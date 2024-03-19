/**
 * @file rm68120.h
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
#include "disp_def.h"
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

esp_err_t rm68120_init(lv_disp_drv_t* disp_drv);
esp_lcd_panel_handle_t* rm68120_panel_handle();
void rm68120_flush(lv_disp_drv_t* disp_drv, const lv_area_t* area,
                   lv_color_t* color_p);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
