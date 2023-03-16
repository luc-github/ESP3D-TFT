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

#include "esp3d_log.h"
#include "esp_freertos_hooks.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "tasks_def.h"
#include "version.h"

#define LV_TICK_PERIOD_MS 10
#define STACKDEPTH NETWORK_STACK_DEPTH
#define TASKPRIORITY 0
#define TASKCORE 1

/**********************
 *  STATIC PROTOTYPES
 **********************/

ESP3DTftUi esp3dTftui;

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_application(void);

static void lv_tick_task(void *arg) {
  (void)arg;

  lv_tick_inc(LV_TICK_PERIOD_MS);
}

static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_btn_red;
static lv_obj_t *labelSlider;

static void slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);

  /*Refresh the text*/
  lv_label_set_text_fmt(labelSlider, "%" LV_PRId32,
                        lv_slider_get_value(slider));
  lv_obj_align_to(labelSlider, slider, LV_ALIGN_OUT_TOP_MID, 0,
                  -15); /*Align top of the slider*/
}

static lv_color_t darken(const lv_color_filter_dsc_t *dsc, lv_color_t color,
                         lv_opa_t opa) {
  LV_UNUSED(dsc);
  return lv_color_darken(color, opa);
}

static void style_init(void) {
  /*Create a simple button style*/
  lv_style_init(&style_btn);
  lv_style_set_radius(&style_btn, 10);
  lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, lv_palette_lighten(LV_PALETTE_GREY, 3));
  lv_style_set_bg_grad_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);

  lv_style_set_border_color(&style_btn, lv_color_black());
  lv_style_set_border_opa(&style_btn, LV_OPA_20);
  lv_style_set_border_width(&style_btn, 2);

  lv_style_set_text_color(&style_btn, lv_color_black());

  /*Create a style for the pressed state.
   *Use a color filter to simply modify all colors in this state*/
  static lv_color_filter_dsc_t color_filter;
  lv_color_filter_dsc_init(&color_filter, darken);
  lv_style_init(&style_btn_pressed);
  lv_style_set_color_filter_dsc(&style_btn_pressed, &color_filter);
  lv_style_set_color_filter_opa(&style_btn_pressed, LV_OPA_20);

  /*Create a red style. Change only some colors.*/
  lv_style_init(&style_btn_red);
  lv_style_set_bg_color(&style_btn_red, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_color(&style_btn_red,
                             lv_palette_lighten(LV_PALETTE_RED, 3));
}

static void create_application(void) {
  /*Initialize the style*/
  style_init();

  /*Create a button and use the new styles*/
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  /* Remove the styles coming from the theme
   * Note that size and position are also stored as style properties
   * so lv_obj_remove_style_all will remove the set size and position too */
  lv_obj_remove_style_all(btn);
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

  /*Add a label to the button*/
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  /*Create another button and use the red style too*/
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_remove_style_all(btn2); /*Remove the styles coming from the theme*/
  lv_obj_set_pos(btn2, 10, 80);
  lv_obj_set_size(btn2, 120, 50);
  lv_obj_add_style(btn2, &style_btn, 0);
  lv_obj_add_style(btn2, &style_btn_red, 0);
  lv_obj_add_style(btn2, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_radius(btn2, LV_RADIUS_CIRCLE, 0); /*Add a local style too*/

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Button 2");
  lv_obj_center(label);
  lv_obj_align_to(btn2, btn, LV_ALIGN_LEFT_MID, 140, 0);

  lv_obj_t *slider = lv_slider_create(lv_scr_act());
  lv_obj_set_width(slider, 200); /*Set the width*/

  lv_obj_center(slider);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL); /*Assign an event function*/

  /*Create a label above the slider*/
  labelSlider = lv_label_create(lv_scr_act());
  lv_label_set_text(labelSlider, "0");
  lv_obj_align_to(labelSlider, slider, LV_ALIGN_OUT_TOP_MID, 0,
                  -15); /*Align top of the slider*/

  lv_obj_t *btnTheme;
  lv_obj_t *labelTheme;

  btnTheme = lv_btn_create(lv_scr_act());
  labelTheme = lv_label_create(btnTheme);
  lv_label_set_text(labelTheme, "Original theme");
  lv_obj_center(labelTheme);

  lv_obj_align_to(btnTheme, slider, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  15); /*Align top of the slider*/
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) {
  (void)pvParameter;
  xGuiSemaphore = xSemaphoreCreateMutex();

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

  create_application();

  while (1) {
    /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Try to take the semaphore, call lvgl related function on success */
    if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
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

bool ESP3DTftUi::end() { return true; }
