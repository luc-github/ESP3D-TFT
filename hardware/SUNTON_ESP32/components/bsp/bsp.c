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
#endif

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
  #if DISP_ST7796
    static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
  #elif DISP_ILI9341
    static void disp_flush_ready();
  #endif
  static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
  #if TOUCH_XPT2046
    static uint16_t touch_spi_read_reg12(uint8_t reg);
  #endif
  static void lv_touch_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
  static lv_disp_drv_t disp_drv;
  #if DISP_ST7796
    static esp_lcd_panel_handle_t disp_panel;
  #endif
  #if TOUCH_GT911
    static i2c_bus_handle_t i2c_bus_handle = NULL;
  #elif TOUCH_XPT2046 && !TOUCH_SW_SPI
    static spi_device_handle_t touch_spi;
  #endif
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
#if SHARED_SPI
  esp3d_log("Initializing SPI master (display,touch)...");
  spi_bus_init(SHARED_SPI_HOST, SHARED_SPI_MISO, SHARED_SPI_MOSI, SHARED_SPI_CLK,
               DISP_BUF_SIZE_BYTES, 1, -1, -1);
#else
  esp3d_log("Initializing SPI master (display)...");
  spi_bus_init(DISP_SPI_HOST, -1, DISP_SPI_MOSI, DISP_SPI_CLK,
               DISP_BUF_SIZE_BYTES, 1, -1, -1);
#endif

esp3d_log("Attaching display panel to SPI bus...");
#if DISP_ST7796  
  esp_lcd_panel_io_handle_t disp_io_handle;
  disp_spi_cfg.on_color_trans_done = disp_flush_ready;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
      (esp_lcd_spi_bus_handle_t)DISP_SPI_HOST, &disp_spi_cfg, &disp_io_handle));
#else
  disp_spi_add_device(DISP_SPI_HOST, &disp_spi_cfg, disp_flush_ready);
#endif

#if TOUCH_XPT2046
  #if TOUCH_SW_SPI
    esp3d_log("Initializing Software SPI master (touch)...");
    sw_spi_init(&touch_spi_cfg);
  #else
    esp3d_log("Attaching touch controller to SPI bus...");
    ESP_ERROR_CHECK(spi_bus_add_device(TOUCH_SPI_HOST, &touch_spi_cfg, &touch_spi));
  #endif
#endif

  /* Display panel initialization */
  esp3d_log("Initializing display...");
#if DISP_ST7796
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(disp_io_handle, &disp_panel_cfg, &disp_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(disp_panel));
  ESP_ERROR_CHECK(esp_lcd_panel_init(disp_panel));
  //ESP_ERROR_CHECK(esp_lcd_panel_invert_color(disp_panel, true));
  #if DISP_LANDSCAPE
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(disp_panel, true));
  #endif
  #if DISP_MIRRORED
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(disp_panel, true, true));
  #endif
#elif DISP_ILI9341
  ili9341_init(&ili9341_cfg);  
  ili9341_set_orientation(DISP_ORIENTATION);
#endif

#if TOUCH_GT911
  /* i2c controller initialization */
  esp3d_log("Initializing I2C controller...");
  i2c_bus_handle = i2c_bus_create(I2C_PORT_NUMBER, &i2c_cfg);
  if (i2c_bus_handle == NULL) {
    esp3d_log_e("I2C bus initialization failed!");    
  }
#endif

  /* Touch controller initialization */
  esp3d_log("Initializing touch controller...");
  bool has_touch_init = true;
#if TOUCH_GT911
  if (i2c_bus_handle == NULL) {
    has_touch_init = false;
  } else if (gt911_init(i2c_bus_handle, &gt911_cfg) != ESP_OK) {
    esp3d_log_e("Touch controller initialization failed!");
    has_touch_init = false;
  }
#elif TOUCH_XPT2046  
  xpt2046_cfg.read_reg12_fn = touch_spi_read_reg12;
  ESP_ERROR_CHECK(xpt2046_init(&xpt2046_cfg));
#endif

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
#endif
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

#if DISP_ST7796
  static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_disp_flush_ready(&disp_drv);
    return false;
  }
#elif DISP_ILI9341
  static void disp_flush_ready() {
    lv_disp_flush_ready(&disp_drv);
  }
#endif

static void lv_disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
  #if DISP_ST7796
    esp_lcd_panel_draw_bitmap(disp_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
  #elif DISP_ILI9341
    ili9341_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);  
  #endif
}

#if TOUCH_XPT2046
  #define MAP(n,min,max,range)  (uint32_t)((uint32_t)((n > min) ? (n-min) : 0) * range) / (max - min);

  static uint16_t touch_spi_read_reg12(uint8_t reg) {
    uint16_t reg_val;
  #if TOUCH_SW_SPI
    reg_val = sw_spi_read_reg16(reg);    
  #else
    uint8_t data[2];
    spi_transaction_t t = {
        .length = sizeof(reg) + 16,
        .rxlength = 16,
        .cmd = reg,
        .rx_buffer = data,
        .flags = 0
    };
    ESP_ERROR_CHECK(spi_device_transmit(touch_spi, &t));
    reg_val = ((uint16_t)data[0] << 8) | data[1];
  #endif
    return reg_val >> 3;
  }
#endif

static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static uint16_t last_x, last_y;
  #if TOUCH_GT911
    gt911_data_t touch_data = gt911_read();
  #elif TOUCH_XPT2046
    xpt2046_data_t touch_data = xpt2046_read(); 
  #endif
  if (touch_data.is_pressed) {
    #if TOUCH_XPT2046
      last_x = MAP(touch_data.x, TOUCH_X_MIN, TOUCH_X_MAX, DISP_HOR_RES_MAX);
      last_y = MAP(touch_data.y, TOUCH_Y_MIN, TOUCH_Y_MAX, DISP_VER_RES_MAX);
    #else
      last_x = touch_data.x;
      last_y = touch_data.y;    
    #endif
    esp3d_log("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif // ESP3D_DISPLAY_FEATURE
