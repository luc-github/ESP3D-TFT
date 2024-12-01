/*
  esp3d_tft

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

#include "esp3d_tft_ui.h"

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_values.h"
#include "esp3d_version.h"
#include "esp_freertos_hooks.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "rendering/esp3d_rendering_client.h"
#include "tasks_def.h"

#define STACKDEPTH UI_STACK_DEPTH
#define TASKPRIORITY UI_TASK_PRIORITY
#define TASKCORE UI_TASK_CORE

/**********************
 *  STATIC PROTOTYPES
 **********************/

ESP3DTftUi esp3dTftui;
#if !LV_TICK_CUSTOM
static void lv_tick_task(void *arg);
#endif
static void guiTask(void *pvParameter);
extern void create_application(void);
#if !LV_TICK_CUSTOM
static void lv_tick_task(void *arg) {
  (void)arg;

  lv_tick_inc(LV_TICK_PERIOD_MS);
}
#endif  // !LV_TICK_CUSTOM

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) {
  (void)pvParameter;
  xGuiSemaphore = xSemaphoreCreateMutex();
#if !LV_TICK_CUSTOM
  /* Create and start a periodic timer interrupt to call lv_tick_inc */
  const esp_timer_create_args_t periodic_timer_args = {
      .callback = &lv_tick_task,
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "periodic_gui",
      .skip_unhandled_events = false};
  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
#endif  // !LV_TICK_CUSTOM
  create_application();

  while (1) {
    /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
    esp3d_hal::wait(10);

    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
      esp3dTftValues.handle();
      lv_task_handler();
      xSemaphoreGive(xGuiSemaphore);
    }
  }

  /* A task should NEVER return */
  vTaskDelete(NULL);
}

ESP3DTftUi::ESP3DTftUi() {}

ESP3DTftUi::~ESP3DTftUi() {}

bool ESP3DTftUi::begin() {
  if (!renderingClient.begin()) {
    esp3d_log_e("Rendering client not started");
    return false;
  }

  // Ui creation
  TaskHandle_t xHandle = NULL;
  BaseType_t res = xTaskCreatePinnedToCore(guiTask, "tftUI", STACKDEPTH, NULL,
                                           TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created UI Task");
    return true;
  } else {
    esp3d_log_e("UI Task creation failed");
    return false;
  }
}

void ESP3DTftUi::handle() {}

bool ESP3DTftUi::end() {
  // TODO : stop TFT task and delete it
  renderingClient.end();
  return true;
}
