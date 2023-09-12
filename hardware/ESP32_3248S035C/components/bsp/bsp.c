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
#include "i2c_def.h"
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
static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void lv_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
static lv_disp_drv_t disp_drv;
static esp_lcd_panel_handle_t disp_panel;
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

  esp3d_log("Attaching display panel to SPI bus...");
  esp_lcd_panel_io_handle_t disp_io_handle;
  disp_spi_cfg.on_color_trans_done = disp_flush_ready;  
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
      (esp_lcd_spi_bus_handle_t)DISP_SPI_HOST, &disp_spi_cfg, &disp_io_handle));

  /* Display panel initialization */
  esp3d_log("Initializing display...");
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(disp_io_handle, &disp_panel_cfg, &disp_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));
  //ESP_ERROR_CHECK(esp_lcd_panel_invert_color(disp_panel, true));
#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(disp_panel, true));
#endif //DISP_ORIENTATION
#if DISP_ORIENTATION == 1 || DISP_ORIENTATION == 3  // mirrored
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(disp_panel, true, true));
#endif //DISP_ORIENTATION

  /* i2c controller initialization */
  esp3d_log("Initializing i2C controller...");
  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &i2c_cfg);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus initialization failed!");
    return ESP_FAIL;
  }

  /* Touch controller initialization */
  esp3d_log("Initializing touch controller...");
  bool has_touch_init = true;
  if (gt911_init(i2c_bus_handle, &gt911_cfg) != ESP_OK) {
    esp3d_log_e("Touch controller initialization failed!");
    has_touch_init = false;
  }

  disp_backlight_set(bcklt_handle, DISP_BCKL_DEFAULT_DUTY);

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

  if (has_touch_init) {
    /* Register the touch input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_touch_read;
    lv_indev_drv_register(&indev_drv);
  }
#endif // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if ESP3D_DISPLAY_FEATURE

static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
  lv_disp_flush_ready(&disp_drv);
  return false;
}

static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
  esp_lcd_panel_draw_bitmap(disp_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
}

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
