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

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

void empty_screen();
void temperatures_screen();
void positions_screen();
void fan_screen();
void speed_screen();
void files_screen();
void settings_screen();

lv_timer_t *main_stream_delay_timer = NULL;
ESP3DScreenType next_screen = ESP3DScreenType::none;

void main_stream_delay_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (main_stream_delay_timer) {
    lv_timer_del(main_stream_delay_timer);
    main_stream_delay_timer = NULL;
  }
  switch (next_screen) {
    case ESP3DScreenType::none:
      empty_screen();
      break;
    case ESP3DScreenType::temperatures:
      temperatures_screen();
      break;
    case ESP3DScreenType::positions:
      positions_screen();
      break;
    case ESP3DScreenType::fan:
      fan_screen();
      break;
    case ESP3DScreenType::speed:
      speed_screen();
      break;
    case ESP3DScreenType::files:
      files_screen();
      break;
    case ESP3DScreenType::settings:
      settings_screen();
      break;

    default:

      break;
  }
  next_screen = ESP3DScreenType::none;
}

lv_obj_t *status_bar(lv_obj_t *screen);

lv_obj_t *create_button(lv_obj_t *container, lv_obj_t *&btn, lv_obj_t *&label,
                        int width = BUTTON_WIDTH, bool center = true) {
  btn = lv_btn_create(container);
  apply_style(btn, ESP3DStyleType::button);
  lv_obj_set_size(btn, width, BUTTON_HEIGHT);
  label = lv_label_create(btn);
  lv_obj_center(label);
  if (center) lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  return label;
}

void event_button_E0_handler(lv_event_t *e) {
  esp3d_log("E0 Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_E1_handler(lv_event_t *e) {
  esp3d_log("E1 Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_Bed_handler(lv_event_t *e) {
  esp3d_log("Bed Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_positions_handler(lv_event_t *e) {
  esp3d_log("Positions Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::positions;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_fan_handler(lv_event_t *e) {
  esp3d_log("Fan Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::fan;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_speed_handler(lv_event_t *e) {
  esp3d_log("Speed Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::speed;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_files_handler(lv_event_t *e) {
  esp3d_log("Files Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::files;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_settings_handler(lv_event_t *e) {
  esp3d_log("Settings Clicked");
  if (main_stream_delay_timer) return;
  next_screen = ESP3DScreenType::settings;
  main_stream_delay_timer =
      lv_timer_create(main_stream_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_resume_handler(lv_event_t *e) { esp3d_log("Resume Clicked"); }

void event_button_pause_handler(lv_event_t *e) { esp3d_log("Pause Clicked"); }

void event_button_stop_handler(lv_event_t *e) { esp3d_log("Pause Clicked"); }

void main_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::main);
  // Screen creation
  esp3d_log("Main screen creation");
  // Background
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  apply_style(ui_main_screen, ESP3DStyleType::main_bg);

  lv_obj_t *ui_status_bar_container = status_bar(ui_main_screen);
  lv_obj_update_layout(ui_status_bar_container);

  // create container for main screen buttons
  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);
  apply_style(ui_container_main_screen, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(ui_status_bar_container));
  lv_obj_align_to(ui_container_main_screen, ui_status_bar_container,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // Add buttons top containers to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_container_main_screen);
  apply_style(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Middle container
  lv_obj_t *ui_middle_container = lv_btn_create(ui_container_main_screen);
  apply_style(ui_middle_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_middle_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_middle_container);

  // Add buttons bottom containers to main container
  lv_obj_t *ui_bottom_buttons_container =
      lv_obj_create(ui_container_main_screen);
  apply_style(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************
  lv_obj_t *label = nullptr;
  lv_obj_t *btn = nullptr;

  // Create button and label for Extruder 0
  label = create_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s\n%s\n%s%s", "160", "260",
                        LV_SYMBOL_HEAT_EXTRUDER, "1");
  lv_obj_add_event_cb(btn, event_button_E0_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for Extruder 1
  label = create_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s\n%s\n%s%s", "20", "60",
                        LV_SYMBOL_HEAT_EXTRUDER, "2");
  lv_obj_add_event_cb(btn, event_button_E1_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for Bed
  label = create_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s\n%s\n%s", "20", "60", LV_SYMBOL_HEAT_BED);
  lv_obj_add_event_cb(btn, event_button_Bed_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for positions
  label = create_button(ui_top_buttons_container, btn, label,
                        BUTTON_WIDTH * 1.5, false);
  lv_label_set_text_fmt(label, "X: %s\nY: %s\nZ: %s", "150.00", "60.00",
                        "15.00");
  lv_obj_add_event_cb(btn, event_button_positions_handler, LV_EVENT_PRESSED,
                      NULL);

  // Create button and label for middle container
  label = lv_label_create(ui_middle_container);
  apply_style(label, ESP3DStyleType::status_bar);
  // lv_obj_t *label = lv_label_create(ui_btn_middle);
  lv_label_set_text_fmt(label, "Progression: %s %%  ETE: 11H 22M 14S\nTest.gco",
                        "50.2");
  lv_obj_center(label);
  lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

  // Create button and label for fan
  label = create_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s\n%s", "100%", LV_SYMBOL_FAN);
  lv_obj_add_event_cb(btn, event_button_fan_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for speed
  label = create_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s\n%s", "100%", LV_SYMBOL_SPEED);
  lv_obj_add_event_cb(btn, event_button_speed_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for pause
  label = create_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_PAUSE);
  lv_obj_add_event_cb(btn, event_button_pause_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for stop
  label = create_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_STOP);
  lv_obj_add_event_cb(btn, event_button_stop_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for settings
  label = create_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_SETTINGS);
  lv_obj_add_event_cb(btn, event_button_settings_handler, LV_EVENT_PRESSED,
                      NULL);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  lv_obj_del(ui_current_screen);
}
