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
#include "esp3d_log.h"
#include "ft5x06.h"
#include "i2c_bus.h"
#include "i2c_def.h"
#include "lvgl.h"
#include "st7796.h"
#include "touch_def.h"
#endif  // ESP3D_DISPLAY_FEATURE
#include "usb_serial.h"


/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
static lv_disp_drv_t disp_drv;
//static esp_lcd_panel_handle_t panel_handle = NULL;
static i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                            .scl_io_num = I2C_SCL_PIN,
                            .sda_io_num = I2C_SDA_PIN,
                            .scl_pullup_en = GPIO_PULLUP_ENABLE,
                            .sda_pullup_en = GPIO_PULLUP_ENABLE,
                            .master.clk_speed = I2C_CLK_SPEED};
#endif  // ESP3D_DISPLAY_FEATURE


/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ESP3D_DISPLAY_FEATURE
//static void lv_touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data);
//void display_flush_ready();
//void st7796_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
//                  lv_color_t *color_p);
#endif // ESP3D_DISPLAY_FEATURE


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the USB functionality of the BSP.
 *
 * This function initializes the USB functionality of the BSP (Board Support Package).
 * It performs any necessary configuration and setup for USB communication.
 *
 * @return esp_err_t Returns ESP_OK if the USB initialization is successful, otherwise returns an error code.
 */
esp_err_t bsp_init_usb(void) {
  /*usb host initialization */
  esp3d_log_d("Initializing usb-serial");
  return usb_serial_create_task();
}

/**
 * @brief Deinitializes the USB functionality of the BSP.
 *
 * This function is responsible for deinitializing the USB functionality of the BSP.
 *
 * @return esp_err_t Returns ESP_OK if the USB deinitialization is successful, otherwise returns an error code.
 */
esp_err_t bsp_deinit_usb(void) {
  esp3d_log_d("Remove usb-serial");
  return usb_serial_deinit();
}

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary components and peripherals required by the BSP.
 *
 * @return esp_err_t Returns ESP_OK on success, or an error code if initialization fails.
 */
esp_err_t bsp_init(void) {
  #if ESP3D_DISPLAY_FEATURE

  // Drivers initialization
  esp3d_log_d("Display buffer size: %d", DISP_BUF_SIZE);

  /* i2c controller initialization */
  esp3d_log_d("Initializing i2C controller");

  if (NULL != i2c_bus_handle) {
    esp3d_log_e("I2C bus already initialized.");
    return ESP_FAIL;
  }

  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &conf);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus failed to be initialized.");
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
  esp3d_log_d("Initializing display controller");
  if (st7796_init(&disp_drv) != ESP_OK) {
    return ESP_FAIL;
  }

  /* Touch controller initialization */
  esp3d_log_d("Initializing touch controller...");
  bool has_touch = true;
  if (ft5x06_init(i2c_bus_handle) != ESP_OK) {
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

   if (has_touch) {
    /* Register an input device */
    static lv_indev_drv_t indev_drv; /*Descriptor of a input device driver*/
    lv_indev_drv_init(&indev_drv);   /*Basic initialization*/
    indev_drv.type =
        LV_INDEV_TYPE_POINTER;         /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = ft5x06_read; /*Set your driver function*/
    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
  }
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}
