/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Created by luc lebosse - luc@tech-hunters - https://github.com/luc-github -
 * 2023-07-20
 */
#include "ek9716.h"

#include <pthread.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "disp_def.h"
#include "driver/gpio.h"
#include "esp3d_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

esp_lcd_panel_handle_t ek9716_panel_handle = NULL;

esp_lcd_panel_handle_t *get_ek9716_panel_handle() {
  return &ek9716_panel_handle;
}

// we use two semaphores to sync the VSYNC event and the LVGL task, to avoid
// potential tearing effect
#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
SemaphoreHandle_t ek9716_sem_vsync_end;
SemaphoreHandle_t ek9716_sem_gui_ready;
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM

static bool ek9716_on_vsync_event(
    esp_lcd_panel_handle_t panel,
    const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data) {
  BaseType_t high_task_awoken = pdFALSE;
#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  if (xSemaphoreTakeFromISR(ek9716_sem_gui_ready, &high_task_awoken) ==
      pdTRUE) {
    xSemaphoreGiveFromISR(ek9716_sem_vsync_end, &high_task_awoken);
  }
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM
  return high_task_awoken == pdTRUE;
}

void ek9716_flush(lv_disp_drv_t *drv, const lv_area_t *area,
                  lv_color_t *color_map) {
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  xSemaphoreGive(ek9716_sem_gui_ready);
  xSemaphoreTake(ek9716_sem_vsync_end, portMAX_DELAY);
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM
  // pass the draw buffer to the driver
  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1,
                            offsety2 + 1, color_map);
  lv_disp_flush_ready(drv);
}

esp_err_t ek9716_init(lv_disp_drv_t *disp_drv) {
  esp3d_log("init ek9716 panel");
  if (ek9716_panel_handle != NULL) {
    esp3d_log_e("ek9716 bus already initialized");
    return ESP_FAIL;
  }

  esp3d_log("Install RGB LCD panel driver");

#if DISP_AVOID_TEAR_EFFECT_WITH_SEM
  esp3d_log("Create semaphores");
  ek9716_sem_vsync_end = xSemaphoreCreateBinary();
  if (!ek9716_sem_vsync_end) {
    esp3d_log_e("Failed to create ek9716_sem_vsync_end");
    return ESP_FAIL;
  }
  ek9716_sem_gui_ready = xSemaphoreCreateBinary();
  if (!ek9716_sem_gui_ready) {
    esp3d_log_e("Failed to create ek9716_sem_gui_ready");
    return ESP_FAIL;
  }
#endif  // DISP_AVOID_TEAR_EFFECT_WITH_SEM

  esp_lcd_rgb_panel_config_t panel_config =
  {.clk_src = LCD_CLK_SRC_DEFAULT,
   .timings =
       {
           .pclk_hz = DISP_CLK_FREQ,
           .h_res = DISP_HOR_RES_MAX,
           .v_res = DISP_VER_RES_MAX,
           .hsync_pulse_width = DISP_HSYNC_PULSE_WIDTH,
           .hsync_back_porch = DISP_HSYNC_BACK_PORSH,
           .hsync_front_porch = DISP_HSYNC_FRONT_PORSH,
           .vsync_pulse_width = DISP_VSYNC_PULSE_WIDTH,
           .vsync_back_porch = DISP_VSYNC_BACK_PORSH,
           .vsync_front_porch = DISP_VSYNC_FRONT_PORSH,
           .flags =
               {
                   .hsync_idle_low = (uint32_t)0,
                   .vsync_idle_low = (uint32_t)0,
                   .de_idle_high = (uint32_t)0,
                   .pclk_active_neg = DISP_PCLK_ACTIVE,
                   .pclk_idle_high = (uint32_t)0,
               },
       },
   .data_width = DISP_BITS_WIDTH,
   .bits_per_pixel = (uint8_t)0,
   .num_fbs = DISP_NUM_FB,
#if DISP_USE_BOUNCE_BUFFER
   .bounce_buffer_size_px =
       sizeof(lv_color_t) * DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 4,
#else
   .bounce_buffer_size_px = (size_t)0,
#endif
   .sram_trans_align = (size_t)0,
   .psram_trans_align = 64,
   .hsync_gpio_num = DISP_HSYNC_PIN,
   .vsync_gpio_num = DISP_VSYNC_PIN,
   .de_gpio_num = DISP_DE_PIN,
   .pclk_gpio_num = DISP_PCLK_PIN,
   .disp_gpio_num = DISP_EN_PIN,

   .data_gpio_nums =
       {
           DISP_D00_PIN,
           DISP_D01_PIN,
           DISP_D02_PIN,
           DISP_D03_PIN,
           DISP_D04_PIN,
           DISP_D05_PIN,
           DISP_D06_PIN,
           DISP_D07_PIN,
           DISP_D08_PIN,
           DISP_D09_PIN,
           DISP_D10_PIN,
           DISP_D11_PIN,
           DISP_D12_PIN,
           DISP_D13_PIN,
           DISP_D14_PIN,
           DISP_D15_PIN,
       },
   .flags = {
       .disp_active_low = (uint32_t)NULL,
       .refresh_on_demand = (uint32_t)NULL,
       .fb_in_psram = DISP_FB_IN_PSRAM,
       .double_fb = (uint32_t)NULL,
       .no_fb = (uint32_t)NULL,
       .bb_invalidate_cache = (uint32_t)NULL,
   } };

  ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &ek9716_panel_handle));

  esp3d_log("Register event callbacks");
  esp_lcd_rgb_panel_event_callbacks_t cbs = {
      .on_vsync = ek9716_on_vsync_event,
  };
  ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(
      ek9716_panel_handle, &cbs, &disp_drv));

  esp3d_log("Initialize RGB LCD panel");
  ESP_ERROR_CHECK(esp_lcd_panel_reset(ek9716_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(ek9716_panel_handle));

  return ESP_OK;
}
