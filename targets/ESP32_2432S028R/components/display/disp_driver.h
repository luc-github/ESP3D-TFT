/**
 * @file disp_driver.h
 */

#ifndef DISP_DRIVER_H
#define DISP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "ili9341.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* Initialize display */
void *disp_driver_init(void);

/* Display flush callback */
void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_DRIVER_H*/
