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

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static bool disp_on_vsync_event(
    esp_lcd_panel_handle_t panel,
    const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data);
static void lv_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                          lv_color_t *color_p);
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if ESP3D_DISPLAY_FEATURE
static i2c_bus_handle_t i2c_bus_handle = NULL;
static lv_disp_drv_t disp_drv;
static esp_lcd_panel_handle_t disp_panel;

#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
// We use two semaphores to sync the VSYNC event and the LVGL task, to avoid
// potential tearing effect
static SemaphoreHandle_t _sem_vsync_end;
static SemaphoreHandle_t _sem_gui_ready;
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM
#endif  // ESP3D_DISPLAY_FEATURE

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary components and peripherals required
 * by the BSP.
 *
 * @return ESP_OK if the BSP initialization is successful, otherwise an error
 * code.
 */
esp_err_t bsp_init(void) {
#if ESP3D_DISPLAY_FEATURE
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

  /* Display panel initialization */
  esp3d_log("Initializing display...");
  if (esp_lcd_new_panel_st7262(&disp_panel_cfg, &disp_panel) != ESP_OK) {
    esp3d_log_e("Failed to create new RGB panel");
    return ESP_FAIL;
  }

#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  esp3d_log("Create semaphores");
  _sem_vsync_end = xSemaphoreCreateBinary();
  if (!_sem_vsync_end) {
    esp3d_log_e("Failed to create st7262_sem_vsync_end");
    return ESP_FAIL;
  }
  _sem_gui_ready = xSemaphoreCreateBinary();
  if (!_sem_gui_ready) {
    esp3d_log_e("Failed to create st7262_sem_gui_ready");
    return ESP_FAIL;
  }
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM

  esp3d_log("Register VSync event callback");
  esp_lcd_rgb_panel_event_callbacks_t cbs = {
      .on_vsync = disp_on_vsync_event,
  };
  if (esp_lcd_rgb_panel_register_event_callbacks(disp_panel, &cbs, &disp_drv) !=
      ESP_OK) {
    esp3d_log_e("Failed to register VSync event callback");
    return ESP_FAIL;
  }

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

  // enable display backlight
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
  void *buf1 = NULL;
  void *buf2 = NULL;
#if DISP_NUM_FB == 2
  esp3d_log("Use panel frame buffers as LVGL draw buffers");
  if (esp_lcd_rgb_panel_get_frame_buffer(disp_panel, 2, &buf1, &buf2) !=
      ESP_OK) {
    esp3d_log_e("Failed to get panel frame buffers");
    return ESP_FAIL;
  }
#else
  esp3d_log("Allocate LVGL draw buffer");
  buf1 = heap_caps_malloc(DISP_BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM);
  if (buf1 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 1");
    return ESP_FAIL;
  }
#if DISP_USE_DOUBLE_BUFFER
  esp3d_log("Allocate 2nd LVGL draw buffer");
  buf2 = heap_caps_malloc(DISP_BUF_SIZE_BYTES, MALLOC_CAP_SPIRAM);
  if (buf2 == NULL) {
    esp3d_log_e("Failed to allocate LVGL draw buffer 2");
    return ESP_FAIL;
  }
#endif  // DISP_USE_DOUBLE_BUFFER
#endif  // DISP_NUM_FB == 2
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_BUF_SIZE);

  /* Register the display device */
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = lv_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.hor_res = DISP_HOR_RES_MAX;
  disp_drv.ver_res = DISP_VER_RES_MAX;
#if DISP_NUM_FB == 2
  // The full_refresh mode can maintain the synchronization between the two
  // frame buffers
  disp_drv.full_refresh = true;
#else
  disp_drv.full_refresh = false;
#endif
  lv_disp_drv_register(&disp_drv);

  if (has_touch_init) {
    /* Register the touch input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lv_touch_read;
    lv_indev_drv_register(&indev_drv);
  }
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**
 * @brief Accesses the file system.
 *
 * This function is responsible for accessing the file system.
 *
 * @return esp_err_t Returns ESP_OK if the file system access is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_accessFs(void) {
#if ESP3D_DISPLAY_FEATURE
  esp_err_t ret = esp_lcd_rgb_panel_set_pclk(disp_panel, DISP_PATCH_FS_FREQ);
  vTaskDelay(pdMS_TO_TICKS(DISP_PATCH_FS_DELAY));
  return ret;
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**
 * @brief Releases the file system resources used by the BSP.
 *
 * This function releases the file system resources used by the BSP (Board
 * Support Package). It is responsible for freeing any allocated memory and
 * closing any open file handles related to the file system.
 *
 * @return `ESP_OK` if the file system resources are successfully released, or
 * an error code if an error occurs.
 */
esp_err_t bsp_releaseFs(void) {
#if ESP3D_DISPLAY_FEATURE
  esp_err_t ret = esp_lcd_rgb_panel_set_pclk(disp_panel, DISP_CLK_FREQ);
  vTaskDelay(pdMS_TO_TICKS(DISP_PATCH_FS_DELAY));
  return ret;
#endif  // ESP3D_DISPLAY_FEATURE
  return ESP_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#if ESP3D_DISPLAY_FEATURE

/**
 * @brief This function is called when a display on vsync event occurs. It is a
 * static function that takes an `esp_lcd_panel_handle_t` panel, a pointer to
 * `esp_lcd_rgb_panel_event_data_t` event_data, and a `void*` user_data as
 * parameters.
 *
 * @param panel The handle to the LCD panel.
 * @param event_data Pointer to the event data associated with the vsync event.
 * @param user_data User-defined data passed to the callback function.
 *
 * @return `true` if the event was handled successfully, `false` otherwise.
 */
static bool disp_on_vsync_event(
    esp_lcd_panel_handle_t panel,
    const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data) {
  BaseType_t high_task_awoken = pdFALSE;
#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  if (xSemaphoreTakeFromISR(_sem_gui_ready, &high_task_awoken) == pdTRUE) {
    xSemaphoreGiveFromISR(_sem_vsync_end, &high_task_awoken);
  }
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM
  return high_task_awoken == pdTRUE;
}

/**
 * @brief Flushes the display with the provided color data within the specified
 * area.
 *
 * This function is responsible for updating the display with the provided color
 * data within the specified area. It is called by the LVGL display driver.
 *
 * @param disp_drv Pointer to the LVGL display driver structure.
 * @param area Pointer to the area to be updated on the display.
 * @param color_p Pointer to the color data array.
 */
static void lv_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                          lv_color_t *color_p) {
#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  xSemaphoreGive(_sem_gui_ready);
  xSemaphoreTake(_sem_vsync_end, portMAX_DELAY);
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM
  esp_lcd_panel_draw_bitmap(disp_panel, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, color_p);
  lv_disp_flush_ready(disp_drv);
}

/**
 * @brief This function is responsible for reading touch input data and updating
 * the LVGL input device data structure.
 *
 * @param drv Pointer to the LVGL input device driver.
 * @param data Pointer to the LVGL input device data structure.
 */
static void lv_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static uint16_t last_x, last_y;
  gt911_data_t touch_data = gt911_read();
  if (touch_data.is_pressed) {
    // Touch seems to have a resolution of 480x272 for some reason.
    // So, we need to map the touch coords to the display coords.
    last_x = touch_data.x * DISP_HOR_RES_MAX / get_gt911_x_max();
    last_y = touch_data.y * DISP_VER_RES_MAX / get_gt911_y_max();
    esp3d_log("Touch x=%d, y=%d", last_x, last_y);
  }
  data->point.x = last_x;
  data->point.y = last_y;
  data->state = touch_data.is_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

#endif
