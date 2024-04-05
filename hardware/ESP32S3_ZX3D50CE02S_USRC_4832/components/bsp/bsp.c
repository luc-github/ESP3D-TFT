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
#include "i2c_def.h"
#include "lvgl.h"
#include "touch_def.h"
#endif  // ESP3D_DISPLAY_FEATURE
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial.h"
#endif //ESP3D_USB_SERIAL_FEATURE

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
static lv_disp_drv_t disp_drv;
static esp_lcd_panel_handle_t panel_handle = NULL;
#endif  // ESP3D_DISPLAY_FEATURE

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
void display_flush_ready();
void st7796_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                  lv_color_t *color_p);
#endif  // ESP3D_DISPLAY_FEATURE

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the USB functionality of the BSP.
 *
 * This function initializes the USB functionality of the BSP (Board Support
 * Package). It performs any necessary configuration and setup for USB
 * communication.
 *
 * @return esp_err_t Returns ESP_OK if the USB initialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_init_usb(void) {
  /*usb host initialization */
  esp3d_log("Initializing usb-serial");
  return usb_serial_create_task();
}

/**
 * @brief Deinitializes the USB functionality of the BSP.
 *
 * This function is responsible for deinitializing the USB functionality of the
 * BSP.
 *
 * @return esp_err_t Returns ESP_OK if the USB deinitialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_deinit_usb(void) {
  esp3d_log("Remove usb-serial");
  return usb_serial_deinit();
}

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary components and peripherals required
 * by the BSP.
 *
 * @return esp_err_t Returns ESP_OK on success, or an error code if
 * initialization fails.
 */
esp_err_t bsp_init(void) {
#if ESP3D_DISPLAY_FEATURE

  /* Display backlight initialization */
  disp_backlight_t *bcklt_handle = NULL;
  esp3d_log("Initializing display backlight...");
  esp_err_t err = disp_backlight_create(&disp_bcklt_cfg, &bcklt_handle);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to initialize display backlight");
    return err;
  }
  if (disp_backlight_set(bcklt_handle, 0) != ESP_OK) {
    esp3d_log_e("Failed to set display backlight");
    return ESP_FAIL;
  }
  // Drivers initialization
  esp3d_log("Display buffer size: %d", DISP_BUF_SIZE);

  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller");

  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller...");
  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &i2c_cfg);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus initialization failed!");
    return ESP_FAIL;
  }
#endif  // ESP3D_DISPLAY_FEATURE

  // NOTE:
  // this location allows usb-host driver to be installed - later it will failed
  // Do not know why...
  if (usb_serial_init() != ESP_OK) {
    return ESP_FAIL;
  }
#if ESP3D_DISPLAY_FEATURE
  /* Display controller initialization */
  esp3d_log("Initializing display controller");
  if (st7796_init(&st7796_cfg, &panel_handle, (void *)display_flush_ready) !=
      ESP_OK) {
    return ESP_FAIL;
  }

  /* Touch controller initialization */
  esp3d_log("Initializing touch controller...");
  bool has_touch = true;
  if (ft5x06_init(i2c_bus_handle, &ft5x06_cfg) != ESP_OK) {
    esp3d_log_e("Touch controller initialization failed!");
    has_touch = false;
  }

  // enable display backlight
  err = disp_backlight_set(bcklt_handle, DISP_BCKL_DEFAULT_DUTY);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to set display backlight");
    return err;
  }

  // Lvgl initialization
  lv_init();

  // Lvgl setup
  esp3d_log("Setup Lvgl");
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(
      DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }

  /* Use double buffered when not working with monochrome displays */
  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(
      DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  if (buf2 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 2");
    return ESP_FAIL;
  }

  static lv_disp_draw_buf_t draw_buf;

  uint32_t size_in_px = DISP_BUF_SIZE;

  /* Initialize the working buffer depending on the selected display.*/
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);

  lv_disp_drv_init(&disp_drv);      /*Basic initialization*/
  disp_drv.flush_cb = st7796_flush; /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;    /*Assign the buffer to the display*/
  disp_drv.hor_res =
      st7796_cfg.hor_res; /*Set the horizontal resolution of the display*/
  disp_drv.ver_res =
      st7796_cfg.ver_res; /*Set the vertical resolution of the display*/
  disp_drv.user_data = panel_handle;
  lv_disp_drv_register(&disp_drv); /*Finally register the driver*/

  if (has_touch) {
    /* Register an input device */
    static lv_indev_drv_t indev_drv; /*Descriptor of a input device driver*/
    lv_indev_drv_init(&indev_drv);   /*Basic initialization*/
    indev_drv.type =
        LV_INDEV_TYPE_POINTER;         /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = lv_touch_read; /*Set your driver function*/
    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
  }
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if ESP3D_DISPLAY_FEATURE

/**
 * @brief Indicates that the display flush is ready.
 *
 * This function is called to indicate that the display flush operation has
 * completed and the display is ready for the next operation.
 */
void display_flush_ready() { lv_disp_flush_ready(&disp_drv); }

/**
 * Flushes the specified area of the display with the given color data.
 *
 * @param disp_drv Pointer to the display driver structure.
 * @param area     Pointer to the area to be flushed.
 * @param color_p  Pointer to the color data.
 */
void st7796_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
          lv_color_t *color_p) {
  esp_lcd_panel_handle_t panel_handle =
      (esp_lcd_panel_handle_t)disp_drv->user_data;

  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, color_p);
}

/**
 * Reads touch input for the LVGL input device driver.
 *
 * @param drv The LVGL input device driver.
 * @param data The LVGL input device data.
 */
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static uint16_t last_x, last_y;
  ft5x06_data_t touch_data = ft5x06_read();
  if (touch_data.is_pressed) {
    last_x = touch_data.x;
    last_y = touch_data.y;
    esp3d_log("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif  // ESP3D_DISPLAY_FEATURE
