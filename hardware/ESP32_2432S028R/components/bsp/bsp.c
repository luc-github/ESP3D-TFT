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
#include "disp_spi.h"
#include "esp_lcd_backlight.h"
#include "ili9341.h"
#include "lvgl.h"
#include "xpt2046.h"

#endif  // ESP3D_DISPLAY_FEATURE
#include "spi_bus.h"

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t bsp_init(void) {
#if ESP3D_DISPLAY_FEATURE
  // Driver initialization
  esp3d_log("Display buffer size: %d", DISP_BUF_SIZE);

  /* Display controller initialization */
  esp3d_log("Initializing SPI master for display");

  spi_driver_init(DISP_SPI_HOST, DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
                  DISP_SPI_BUS_MAX_TRANSFER_SZ, 1, DISP_SPI_IO2, DISP_SPI_IO3);

  esp3d_log("Initializing display controller");
  disp_spi_add_device(DISP_SPI_HOST);
  ili9341_init();
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
  disp_backlight_set(bckl_handle, DISP_BCKL_DEFAULT_DUTY);
#endif
  /* Touch controller initialization */
  esp3d_log("Initializing touch controller");
  xpt2046_init();

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
  lv_color_t *buf2 = NULL;
#if DISP_USE_DOUBLE_BUFFER
  buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t),
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

  static lv_disp_drv_t disp_drv;     /*Descriptor of a display driver*/
  lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
  disp_drv.flush_cb = ili9341_flush; /*Set your driver function*/
  disp_drv.draw_buf = &draw_buf;     /*Assign the buffer to the display*/
  disp_drv.hor_res =
      DISP_HOR_RES_MAX; /*Set the horizontal resolution of the display*/
  disp_drv.ver_res =
      DISP_VER_RES_MAX; /*Set the vertical resolution of the display*/
  lv_disp_drv_register(&disp_drv); /*Finally register the driver*/

  /* Register an input device */
  static lv_indev_drv_t indev_drv; /*Descriptor of a input device driver*/
  lv_indev_drv_init(&indev_drv);   /*Basic initialization*/
  indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = xpt2046_read;       /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);      /*Finally register the driver*/
#endif                                    // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}
