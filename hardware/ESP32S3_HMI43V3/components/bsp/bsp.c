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
#include "tca9554_def.h"
#include "touch_def.h"
#endif  // ESP3D_DISPLAY_FEATURE

#include "usb_serial.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static void lv_touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data);
void display_flush_ready();
void rm68120_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p);
#endif // ESP3D_DISPLAY_FEATURE

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
static lv_disp_drv_t disp_drv;
 static esp_lcd_panel_handle_t disp_panel_handle = NULL;
#endif  // ESP3D_DISPLAY_FEATURE



/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief Initializes the USB functionality of the BSP.
 *
 * This function initializes the USB functionality of the BSP (Board Support
 * Package). It performs any necessary setup and configuration for USB
 * communication.
 *
 * @return esp_err_t Returns `ESP_OK` if the USB initialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_init_usb(void) {
  /*usb host initialization */
  esp3d_log_d("Initializing usb-serial");
  return usb_serial_create_task();
}

/**
 * @brief Deinitializes the USB functionality of the BSP.
 *
 * This function is responsible for deinitializing the USB functionality of the
 * BSP.
 *
 * @return esp_err_t Returns `ESP_OK` if the USB deinitialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_deinit_usb(void) {
  esp3d_log_d("Remove usb-serial");
  return usb_serial_deinit();
}

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary hardware and peripherals for the BSP.
 *
 * @return esp_err_t Returns ESP_OK if the initialization is successful,
 * otherwise an error code.
 */
esp_err_t bsp_init(void) {
#if ESP3D_DISPLAY_FEATURE
  static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/

  // Drivers initialization
  esp3d_log_d("Display buffer size: %d", DISP_BUF_SIZE);

  /* i2c controller initialization */
  esp3d_log_d("Initializing i2C controller...");
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
  /* tca9554 controller initialization */
  esp3d_log_d("Initializing tca9554 controller");
  if (tca9554_init(i2c_bus_handle, &tca9554_cfg) != ESP_OK) {
    esp3d_log_e("TCA9554 initialization failed!");
    return ESP_FAIL;
  }

  esp3d_log_d("panel handle before init %p",disp_panel_handle );

  /* Display controller initialization */
  esp3d_log_d("Initializing display controller");
  if (esp_lcd_new_panel_rm68120(&rm68120_cfg, &disp_panel_handle, (void*)display_flush_ready)!=ESP_OK){
    esp3d_log_e("RM68120 initialization failed!");
    return ESP_FAIL;
  }
   esp3d_log_d("panel handle afeter init %p",disp_panel_handle );
  /* Touch controller initialization */
  esp3d_log_d("Initializing touch controller...");
  bool has_touch = true;
  if (ft5x06_init(i2c_bus_handle, &ft5x06_cfg) != ESP_OK) {
    esp3d_log_e("Touch controller initialization failed!");
    has_touch = false;
  }

  // Lvgl initialization
  lv_init();

  // Lvgl setup
  esp3d_log_d("Setup Lvgl");
  lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(
      DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }

  /* Use double buffered when not working with monochrome displays */
  lv_color_t* buf2 = NULL;
#if DISP_USE_DOUBLE_BUFFER
  buf2 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t),
                                       MALLOC_CAP_DMA);
  if (buf2 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 2");
    return ESP_FAIL;
  }
#endif  // DISP_USE_DOUBLE_BUFFER
  static lv_disp_draw_buf_t draw_buf;

  uint32_t size_in_px = DISP_BUF_SIZE;

  /* Initialize the working buffer depending on the selected display.*/
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);

 
  lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
  disp_drv.flush_cb = rm68120_flush; /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;     /*Assign the buffer to the display*/
  disp_drv.hor_res = rm68120_cfg.hor_res ; /*Set the horizontal resolution of the display*/
  disp_drv.ver_res =  rm68120_cfg.ver_res; /*Set the vertical resolution of the display*/
  disp_drv.user_data = &disp_panel_handle;

  esp3d_log_d("panel handle assigned %p",disp_drv.user_data );

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

void display_flush_ready(){
  lv_disp_flush_ready(&disp_drv);
}

void rm68120_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p) {
  esp_lcd_panel_handle_t * panel_handle_ptr =
      ((esp_lcd_panel_handle_t *)(disp_drv->user_data));
      esp3d_log_d("panel handle %p",*panel_handle_ptr );

  esp_lcd_panel_draw_bitmap(*panel_handle_ptr, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, color_p);
}

/**
 * Reads touch input for the LVGL input device driver.
 *
 * @param drv The LVGL input device driver.
 * @param data The LVGL input device data.
 */
static void lv_touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {
  static uint16_t last_x, last_y;
  ft5x06_data_t touch_data = ft5x06_read();
  if (touch_data.is_pressed) {
    last_x = touch_data.x;
    last_y = touch_data.y;
    esp3d_log_d("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif  // ESP3D_DISPLAY_FEATURE