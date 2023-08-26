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

#include "fan_screen.h"

#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "main_screen.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

namespace fanScreen {

std::string fan_value = "100";
const char *fan_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t fan_buttons_map_id = 0;

lv_timer_t *fan_screen_delay_timer = NULL;

void fan_screen_delay_timer_cb(lv_timer_t *timer) {
  if (fan_screen_delay_timer) {
    lv_timer_del(fan_screen_delay_timer);
    fan_screen_delay_timer = NULL;
  }
  mainScreen::main_screen();
}

void event_button_fan_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    fan_screen_delay_timer = lv_timer_create(fan_screen_delay_timer_cb,
                                             BUTTON_ANIMATION_DELAY, NULL);
  } else {
    fan_screen_delay_timer_cb(NULL);
  }
}

void fan_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
  }

  else if (code == LV_EVENT_DEFOCUSED) {
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    fan_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    // No idea how to change ta focus to another control
    // every tests I did failed
    // so I refresh all screen ... orz
    fan_value = lv_textarea_get_text(ta);
    fan_screen();
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    fan_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", fan_value.c_str());
  }
}

void fan_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan_value = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan_value);
  int step = atoi(fan_buttons_map[fan_buttons_map_id]);
  fan_int += step;
  fan_value = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan_value.c_str());
}

void fan_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan_value = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan_value);
  int step = atoi(fan_buttons_map[fan_buttons_map_id]);
  fan_int -= step;
  fan_value = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan_value.c_str());
}

void fan_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan_value = lv_textarea_get_text(fan_ta);
  esp3d_log("Ok: %s", fan_value.c_str());
}

void fan_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(fan_ta, "100");
}

void fan_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  fan_buttons_map_id = id;
  esp3d_log("Button %s clicked", fan_buttons_map[id]);
}

void fan_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Fan screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_fan_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::simple_container);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_main_container);
  lv_btnmatrix_set_map(btnm, fan_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, fan_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, fan_matrix_buttons_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);

  lv_obj_t *label = lv_label_create(ui_main_container);
  lv_label_set_text(label, LV_SYMBOL_FAN);
  apply_style(label, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label);
  lv_obj_set_y(label, lv_obj_get_height(ui_main_container) / 2 -
                          lv_obj_get_height(label) / 2);

  lv_obj_t *fan_ta = lv_textarea_create(ui_main_container);
  lv_obj_add_event_cb(fan_ta, fan_ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_textarea_set_accepted_chars(fan_ta, "0123456789");
  lv_textarea_set_max_length(fan_ta, 3);
  lv_textarea_set_one_line(fan_ta, true);
  esp3d_log("Fan value: %s", fan_value.c_str());
  std::string fan_value_init = fan_value;
  lv_textarea_set_text(fan_ta, fan_value_init.c_str());
  lv_obj_set_style_text_align(fan_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(fan_ta, LV_HOR_RES / 6);

  lv_obj_align_to(fan_ta, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  lv_obj_t *btn = symbolButton::create_symbol_button(
      ui_main_container, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn, fan_ta, LV_ALIGN_OUT_TOP_MID, 0,
                  -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, fan_btn_up_event_cb, LV_EVENT_CLICKED, fan_ta);

  btn = symbolButton::create_symbol_button(ui_main_container,
                                           LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn, fan_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, fan_btn_down_event_cb, LV_EVENT_CLICKED, fan_ta);

  label = lv_label_create(ui_main_container);
  lv_label_set_text(label, "%");
  apply_style(label, ESP3DStyleType::bg_label);

  lv_obj_align_to(label, fan_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  btn = symbolButton::create_symbol_button(ui_main_container, LV_SYMBOL_OK);
  lv_obj_align_to(btn, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn, fan_btn_ok_event_cb, LV_EVENT_CLICKED, fan_ta);
  lv_obj_t *btn2 =
      symbolButton::create_symbol_button(ui_main_container, LV_SYMBOL_GAUGE);
  lv_obj_align_to(btn2, btn, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn2, fan_btn_reset_event_cb, LV_EVENT_CLICKED, fan_ta);

  lv_obj_t *fan_kb = lv_keyboard_create(ui_main_container);
  lv_keyboard_set_mode(fan_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(fan_kb, NULL);
  lv_obj_align_to(fan_kb, fan_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(fan_kb);
  lv_obj_set_content_width(fan_kb, LV_HOR_RES - lv_obj_get_x(fan_kb) -
                                       2 * CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(fan_kb, CURRENT_BUTTON_RADIUS_VALUE, LV_PART_MAIN);
  lv_obj_add_flag(fan_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(fan_ta, fan_ta_event_cb, LV_EVENT_ALL, fan_kb);
  esp3dTftui.set_current_screen(ESP3DScreenType::fan);
}
}  // namespace fanScreen