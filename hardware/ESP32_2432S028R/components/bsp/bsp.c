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
#include "lvgl.h"
#include "disp_def.h"
#include "touch_def.h"
#endif  // ESP3D_DISPLAY_FEATURE

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
static void disp_flush_ready();
static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static uint16_t touch_spi_read_reg12(uint8_t reg);
static void lv_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static lv_disp_drv_t disp_drv;
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t bsp_init(void) {
#if ESP3D_DISPLAY_FEATURE  
  /* Display backlight initialization */
  disp_backlight_h bcklt_handle = disp_backlight_new(&disp_bcklt_cfg);
  disp_backlight_set(bcklt_handle, 0);

  /* SPI master initialization */
  esp3d_log("Initializing SPI master (display)...");
  spi_bus_init(DISP_SPI_HOST, -1, DISP_SPI_MOSI, DISP_SPI_CLK,
               DISP_BUF_SIZE_BYTES, 1, -1, -1);

  disp_spi_add_device(DISP_SPI_HOST, &disp_spi_cfg, disp_flush_ready);

  /* Display panel initialization */
  esp3d_log("Initializing display...");
  ili9341_init(&ili9341_cfg);
  //ili9341_set_invert_color(true);
  ili9341_set_orientation(DISP_ORIENTATION);

  /* Touch controller initialization */  
  esp3d_log("Initializing touch controller...");
  sw_spi_init(&touch_spi_cfg);  
  xpt2046_cfg.read_reg12_fn = touch_spi_read_reg12;
  ESP_ERROR_CHECK(xpt2046_init(&xpt2046_cfg));

  disp_backlight_set(bcklt_handle, 100);

  // Lvgl initialization
  esp3d_log("Initializing LVGL...");
  lv_init();
  
  /* Initialize the working buffer(s) depending on the selected display. */
  static lv_disp_draw_buf_t draw_buf;
  esp3d_log("Display buffer size: %1.2f KB", DISP_BUF_SIZE_BYTES / 1024.0);
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE_BYTES, DISP_BUF_MALLOC_TYPE);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }
  lv_color_t *buf2 = NULL;
#if DISP_USE_DOUBLE_BUFFER
  buf2 = (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE_BYTES, DISP_BUF_MALLOC_TYPE);
  if (buf2 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 2");
    return ESP_FAIL;
  }
#endif  // DISP_USE_DOUBLE_BUFFER
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE);

  /* Register the display device */  
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = lv_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.hor_res = DISP_HOR_RES_MAX;
  disp_drv.ver_res = DISP_VER_RES_MAX;
  lv_disp_drv_register(&disp_drv);

  /* Register the touch input device */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lv_touch_read;
  lv_indev_drv_register(&indev_drv);
#endif // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if ESP3D_DISPLAY_FEATURE

static void disp_flush_ready() {
  lv_disp_flush_ready(&disp_drv);
}

static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
  ili9341_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);  
}

static uint16_t touch_spi_read_reg12(uint8_t reg) {
  uint16_t data = sw_spi_read_reg16(reg);
  return data >> 3;
}

#define MAP(n,min,max,range)  (uint32_t)((uint32_t)((n > min) ? (n-min) : 0) * range) / (max - min);

static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static uint16_t last_x, last_y;
  xpt2046_data_t touch_data = xpt2046_read(); 
  if (touch_data.is_pressed) {
    last_x = MAP(touch_data.x, TOUCH_X_MIN, TOUCH_X_MAX, DISP_HOR_RES_MAX);
    last_y = MAP(touch_data.y, TOUCH_Y_MIN, TOUCH_Y_MAX, DISP_VER_RES_MAX);
    esp3d_log("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif
