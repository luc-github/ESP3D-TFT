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

#include "disp_def.h"
#include "ek9716.h"
#include "esp3d_log.h"
#include "esp_lcd_backlight.h"
#include "gt911.h"
#include "i2c_bus.h"
#include "i2c_def.h"
#include "lvgl.h"
#include "touch_def.h"

static i2c_bus_handle_t i2c_bus_handle = NULL;

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
static i2c_config_t conf = {.mode = I2C_MODE_MASTER,
                            .scl_io_num = I2C_SCL_PIN,
                            .sda_io_num = I2C_SDA_PIN,
                            .scl_pullup_en = GPIO_PULLUP_ENABLE,
                            .sda_pullup_en = GPIO_PULLUP_ENABLE,
                            .master.clk_speed = I2C_CLK_SPEED};
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t bsp_init(void) {
  static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/

  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller");

  if (NULL != i2c_bus_handle) {
    esp3d_log_e("I2C bus already initialized.");
    return ESP_FAIL;
  }

  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &conf);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus failed to be initialized.");
    return ESP_FAIL;
  }

#if (defined(DISP_BACKLIGHT_SWITCH) || defined(DISP_BACKLIGHT_PWM))
  const disp_backlight_config_t bckl_config = {
    .gpio_num = DISP_PIN_BCKL,
#if defined DISP_BACKLIGHT_PWM
    .pwm_control = true,
#else
    .pwm_control = false,
#endif
#if defined BACKLIGHT_ACTIVE_LVL
    .output_invert = false,  // Backlight on high
#else
    .output_invert = true,  // Backlight on low
#endif
    .timer_idx = 0,
    .channel_idx =
        0  // @todo this prevents us from having two PWM controlled displays
  };
  disp_backlight_h bckl_handle = disp_backlight_new(&bckl_config);
  disp_backlight_set(bckl_handle, 0);
#endif

  /* Display controller initialization */
  esp3d_log("Initializing display controller");
  if (ek9716_init(&disp_drv) != ESP_OK) {
    return ESP_FAIL;
  }

#if (defined(DISP_BACKLIGHT_SWITCH) || defined(DISP_BACKLIGHT_PWM))
  disp_backlight_set(bckl_handle, DISP_BCKL_DEFAULT_DUTY);
#endif

  /* Touch controller initialization */
  esp3d_log("Initializing touch controller");
  if (gt911_init(i2c_bus_handle) != ESP_OK) {
    return ESP_FAIL;
  }

  // Lvgl initialization
  lv_init();

  // Lvgl setup
  esp3d_log("Setup Lvgl");
  void* buf1 = NULL;
  void* buf2 = NULL;
  static lv_disp_draw_buf_t draw_buf;
  esp_lcd_panel_handle_t* panel_handle = get_ek9716_panel_handle();
#if DISP_NUM_FB == 2
  esp3d_log("Use frame buffers as LVGL draw buffers");
  ESP_ERROR_CHECK(
      esp_lcd_rgb_panel_get_frame_buffer(*panel_handle, 2, &buf1, &buf2));
  // initialize LVGL draw buffers
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2,
                        DISP_HOR_RES_MAX * DISP_VER_RES_MAX);
#else
  esp3d_log("Allocate separate LVGL draw buffers from PSRAM");
  buf1 = heap_caps_malloc(DISP_HOR_RES_MAX * 100 * sizeof(lv_color_t),
                          MALLOC_CAP_SPIRAM);
  if (buf1 == NULL) {
    return ESP_FAIL;
  }

  /* Use double buffered when not working with monochrome displays */
  buf2 = heap_caps_malloc(DISP_HOR_RES_MAX * 100 * sizeof(lv_color_t),
                          MALLOC_CAP_SPIRAM);
  if (buf2 == NULL) {
    return ESP_FAIL;
  }

  uint32_t size_in_px = DISP_HOR_RES_MAX * 100;

  /* Initialize the working buffer depending on the selected display.*/
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);
#endif  // DISP_NUM_FB == 2

  lv_disp_drv_init(&disp_drv);      /*Basic initialization*/
  disp_drv.flush_cb = ek9716_flush; /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;    /*Assign the buffer to the display*/
  disp_drv.hor_res =
      DISP_HOR_RES_MAX; /*Set the horizontal resolution of the display*/
  disp_drv.ver_res =
      DISP_VER_RES_MAX; /*Set the vertical resolution of the display*/
  disp_drv.user_data = *panel_handle;
#if DISP_NUM_FB == 2
  disp_drv.full_refresh =
      true;  // the full_refresh mode can maintain the synchronization between
             // the two frame buffers
#endif
  lv_disp_drv_register(&disp_drv); /*Finally register the driver*/

  /* Register an input device */
  static lv_indev_drv_t indev_drv; /*Descriptor of a input device driver*/
  lv_indev_drv_init(&indev_drv);   /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = gt911_read;         /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);      /*Finally register the driver*/

  return ESP_OK;
}