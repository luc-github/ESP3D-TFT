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
#include "lvgl.h"
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
static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                             esp_lcd_panel_io_event_data_t *edata,
                             void *user_ctx);
static void lv_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                          lv_color_t *color_p);
static uint16_t touch_spi_read_reg12(uint8_t reg);
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static lv_disp_drv_t disp_drv;
static esp_lcd_panel_handle_t disp_panel;
#endif

/**********************
 *      MACROS
 **********************/

#define MAP(n, min, max, range) \
  (uint32_t)((uint32_t)((n > min) ? (n - min) : 0) * range) / (max - min);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary hardware and peripherals required by
 * the BSP.
 *
 * @return esp_err_t Returns `ESP_OK` on success, or an error code if
 * initialization fails.
 */
esp_err_t bsp_init(void) {
  esp_err_t err = ESP_OK;
#if ESP3D_DISPLAY_FEATURE
  /* Display backlight initialization */
  disp_backlight_t *bcklt_handle = NULL;
  esp3d_log("Initializing display backlight...");
  err = disp_backlight_create(&disp_bcklt_cfg, &bcklt_handle);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to initialize display backlight");
    return err;
  }
  if (disp_backlight_set(bcklt_handle, 0) != ESP_OK) {
    esp3d_log_e("Failed to set display backlight");
    return ESP_FAIL;
  }

  /* SPI master initialization */
  esp3d_log("Initializing SPI master (display)...");
  if (spi_bus_init(DISP_SPI_HOST, DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
                   DISP_BUF_SIZE_BYTES, 1, -1, -1) != ESP_OK) {
    esp3d_log_e("SPI master initialization failed!");
    return ESP_FAIL;
  }

  esp3d_log("Attaching display panel to SPI bus...");
  esp_lcd_panel_io_handle_t disp_io_handle;
  disp_spi_cfg.on_color_trans_done = disp_flush_ready;
  if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)DISP_SPI_HOST,
                               &disp_spi_cfg, &disp_io_handle) != ESP_OK) {
    esp3d_log_e("Failed to attach display panel to SPI bus");
    return ESP_FAIL;
  }

  /* Display panel initialization */
  esp3d_log("Initializing display...");
  if (esp_lcd_new_panel_ili9341(disp_io_handle, &disp_panel_cfg, &disp_panel) !=
      ESP_OK) {
    esp3d_log_e("Failed to initialize display");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_reset(disp_panel) != ESP_OK) {
    esp3d_log_e("Failed to reset display");
    return ESP_FAIL;
  }
  if (esp_lcd_panel_init(disp_panel) != ESP_OK) {
    esp3d_log_e("Failed to initialize display");
    return ESP_FAIL;
  }

  // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(disp_panel, true));
#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
  if (esp_lcd_panel_swap_xy(disp_panel, true) != ESP_OK) {
    esp3d_log_e("Failed to swap display orientation");
    return ESP_FAIL;
  }
#endif                                              // DISP_ORIENTATION
#if DISP_ORIENTATION == 1 || DISP_ORIENTATION == 3  // mirrored
  if (esp_lcd_panel_mirror(disp_panel, true, true) != ESP_OK) {
    esp3d_log_e("Failed to mirror display orientation");
    return ESP_FAIL;
  }

#endif  // DISP_ORIENTATION

  /* Touch controller initialization */
  bool has_touch = true;
  // Initialize the sw_spi for touch controller
  if (sw_spi_init(&touch_spi_cfg) != ESP_OK) {
    esp3d_log_e("Failed to initialize sw spi for touch controller");
    has_touch = false;
  }
  if (has_touch) {
    esp3d_log("Initializing touch controller...");
    xpt2046_cfg.read_reg12_fn = touch_spi_read_reg12;

    if (xpt2046_init(&xpt2046_cfg) != ESP_OK) {
      esp3d_log_e("Failed to initialize touch controller");
      has_touch = false;
    } else {
      esp3d_log("Touch controller initialized");
    }
  }

  err = disp_backlight_set(bcklt_handle, DISP_BCKL_DEFAULT_DUTY);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to set display backlight");
    return err;
  }

  // Lvgl initialization
  esp3d_log("Initializing LVGL...");
  lv_init();

  /* Initialize the working buffer(s) depending on the selected display. */
  static lv_disp_draw_buf_t draw_buf;
  esp3d_log("Display buffer size: %1.2f KB", DISP_BUF_SIZE_BYTES / 1024.0);
  lv_color_t *buf1 =
      (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE_BYTES, DISP_BUF_MALLOC_TYPE);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }
  lv_color_t *buf2 = NULL;
#if DISP_USE_DOUBLE_BUFFER
  buf2 =
      (lv_color_t *)heap_caps_malloc(DISP_BUF_SIZE_BYTES, DISP_BUF_MALLOC_TYPE);
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
  if (has_touch) {
    esp3d_log("Touch controller initialized");
    /* Register the touch input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_touch_read;
    lv_indev_drv_register(&indev_drv);
  } else {
    esp3d_log_e(
        "Touch controller not found, touch input device not registered");
  }

#endif  // ESP3D_DISPLAY_FEATURE
  return err;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if ESP3D_DISPLAY_FEATURE

/**
 * @brief Checks if the display flush is ready.
 *
 * This function is used to determine if the display flush is ready to be
 * performed.
 *
 * @param panel_io The handle to the LCD panel I/O.
 * @param edata The event data associated with the LCD panel I/O.
 * @param user_ctx The user context.
 * @return `true` if the display flush is ready, `false` otherwise.
 */
/**
 * @brief Checks if the display flush is ready.
 *
 * This function is used to check if the display flush is ready to be performed.
 *
 * @param panel_io The handle to the LCD panel I/O.
 * @param edata The event data associated with the LCD panel I/O.
 * @param user_ctx The user context.
 * @return `true` if the display flush is ready, `false` otherwise.
 */
static bool disp_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                             esp_lcd_panel_io_event_data_t *edata,
                             void *user_ctx) {
  lv_disp_flush_ready(&disp_drv);
  return false;
}

/**
 * Flushes the display with the specified color data in the given area.
 *
 * @param disp_drv Pointer to the display driver structure.
 * @param area     Pointer to the area to be flushed.
 * @param color_p  Pointer to the color data array.
 */
static void lv_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                          lv_color_t *color_p) {
  esp_lcd_panel_draw_bitmap(disp_panel, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, color_p);
}

/**
 * @brief
 *
 *
 * @param reg The register to read from.
 * @return The 12-bit value read from the register.
 */
static uint16_t touch_spi_read_reg12(uint8_t reg) {
  uint16_t data = sw_spi_read_reg16(reg);
  return data >> 3;
}

/**
 * @brief
 *
 *
 * This function is responsible for reading touch input data and updating the
 * LVGL input device data structure.
 *
 * @param drv Pointer to the LVGL input device driver.
 * @param data Pointer to the LVGL input device data structure.
 */
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
