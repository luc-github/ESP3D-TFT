/**
 * @file touch_driver.h
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "xpt2046.h"


/*********************
*      DEFINES
*********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void touch_driver_init(void);

void touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOUCH_DRIVER_H */

