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
#include "bsp.h"

#include "esp3d_log.h"

#if ESP3D_DISPLAY_FEATURE
#include "disp_def.h"
#include "st7796.h"
#include "i2c_def.h"
#include "lvgl.h"
#include "touch_def.h"
#endif  // ESP3D_DISPLAY_FEATURE

#if  ESP3D_USB_SERIAL_FEATURE
#include "usb_serial.h"
#endif  // ESP3D_USB_SERIAL_FEATURE


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static void lv_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
#endif
/**********************
 *  STATIC VARIABLES
**********************/
 
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
#endif
/**********************
         MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#if  ESP3D_USB_SERIAL_FEATURE
esp_err_t bsp_init_usb(void) {
  //usb host initialization 
  esp3d_log("Initializing usb-serial");
  return usb_serial_create_task();
}

esp_err_t bsp_deinit_usb(void) {
  esp3d_log("Remove usb-serial");
  return usb_serial_deinit();
}
#endif  // ESP3D_USB_SERIAL_FEATURE
esp_err_t bsp_init(void) {
  static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/

  // Drivers initialization
  esp3d_log("Display buffer size: %d", DISP_BUF_SIZE);

  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller");
  if (NULL != i2c_bus_handle) {
    esp3d_log_e("I2C bus already initialized.");
    return ESP_FAIL;
  }
  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller...");
  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &i2c_cfg);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus initialization failed!");
    return ESP_FAIL;
  }


  // NOTE:
  // this location allows usb-host driver to be installed - later it will failed
  // Do not know why...
#if  ESP3D_USB_SERIAL_FEATURE
  if (usb_serial_init() != ESP_OK) {
    return ESP_FAIL;
  }
#endif  // ESP3D_USB_SERIAL_FEATURE



  /* Display controller initialization */
  esp3d_log("Initializing display controller");
  if (st7796_init(&disp_drv) != ESP_OK) {
    return ESP_FAIL;
  }

  /* Touch controller initialization */
  esp3d_log("Initializing touch controller...");
  bool has_touch_init = true;
  if (gt911_init(i2c_bus_handle, &gt911_cfg) != ESP_OK) {
    esp3d_log_e("Touch controller initialization failed!");
    has_touch_init = false;
  }

  // Lvgl initialization
  lv_init();

  // Lvgl setup
  esp3d_log("Setup Lvgl");
  lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(
      DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }

  /* Use double buffered when not working with monochrome displays */
  lv_color_t* buf2 = (lv_color_t*)heap_caps_malloc(
      DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  if (buf2 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 2");
    return ESP_FAIL;
  }

  static lv_disp_draw_buf_t draw_buf;

  uint32_t size_in_px = DISP_BUF_SIZE;

  /* Initialize the working buffer depending on the selected display.*/
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);

  esp_lcd_panel_handle_t* panel_handle = st7796_panel_handle();
  lv_disp_drv_init(&disp_drv);      /*Basic initialization*/
  disp_drv.flush_cb = st7796_flush; /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;    /*Assign the buffer to the display*/
  disp_drv.hor_res =
      DISP_HOR_RES_MAX; /*Set the horizontal resolution of the display*/
  disp_drv.ver_res =
      DISP_VER_RES_MAX; /*Set the vertical resolution of the display*/
  disp_drv.user_data = *panel_handle;
  lv_disp_drv_register(&disp_drv); /*Finally register the driver*/

  if (has_touch_init) {
    /* Register the touch input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_touch_read;
    lv_indev_drv_register(&indev_drv);
  }
  return ESP_OK;
}



/**********************
 *   STATIC FUNCTIONS
 **********************/
#if ESP3D_DISPLAY_FEATURE
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static uint16_t last_x, last_y;
  gt911_data_t touch_data = gt911_read(); 
  if (touch_data.is_pressed) {
    last_x = touch_data.x;
    last_y = touch_data.y;
    esp3d_log("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif