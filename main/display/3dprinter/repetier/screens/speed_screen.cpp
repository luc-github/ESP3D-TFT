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

#include "screens/speed_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/main_screen.h"
#include "rendering/esp3d_rendering_client.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace speedScreen {
std::string speed_value = "100";
const char *speed_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t speed_buttons_map_id = 0;
lv_obj_t *label_current_speed_value = NULL;

lv_timer_t *speed_screen_delay_timer = NULL;

void speed_screen_delay_timer_cb(lv_timer_t *timer) {
  if (speed_screen_delay_timer) {
    lv_timer_del(speed_screen_delay_timer);
    speed_screen_delay_timer = NULL;
  }
  mainScreen::create();
}

void event_button_speed_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (speed_screen_delay_timer) return;
    speed_screen_delay_timer = lv_timer_create(speed_screen_delay_timer_cb,
                                               ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    speed_screen_delay_timer_cb(NULL);
  }
}

void send_gcode_speed() {
  std::string speed_value_str = "M220 S";
  speed_value_str += speed_value;
  renderingClient.sendGcode(speed_value_str.c_str());
}

void speed_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_PRESSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
  } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY ||
             code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    speed_value = lv_textarea_get_text(ta);
    if (std::atoi(speed_value.c_str()) > 300) {
      lv_textarea_set_text(ta, "300");
    }
    if (std::atoi(speed_value.c_str()) < 1) {
      lv_textarea_set_text(ta, "1");
    }
    if (speed_value.length() == 0) {
      lv_textarea_set_text(ta, "1");
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    speed_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", speed_value.c_str());
  }
}

void speed_btn_up_event_cb(lv_event_t *e) {
  esp3d_log("Up");
  lv_obj_t *speed_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string speed = lv_textarea_get_text(speed_ta);
  int speed_int = std::stoi(speed);
  int step = atoi(speed_buttons_map[speed_buttons_map_id]);
  esp3d_log("Step: %d, Speed: %d", step, speed_int);
  speed_int += step;
  if (speed_int > 300) speed_int = 300;
  esp3d_log("new Speed: %d", speed_int);
  speed = std::to_string(speed_int);
  lv_textarea_set_text(speed_ta, speed.c_str());
  send_gcode_speed();
}

void speed_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *speed_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string speed = lv_textarea_get_text(speed_ta);
  int speed_int = std::stoi(speed);
  int step = atoi(speed_buttons_map[speed_buttons_map_id]);
  speed_int -= step;
  if (speed_int < 1) speed_int = 1;
  speed = std::to_string(speed_int);
  lv_textarea_set_text(speed_ta, speed.c_str());
  send_gcode_speed();
}

void speed_btn_ok_event_cb(lv_event_t *e) {
  esp3d_log("Ok clicked");
  send_gcode_speed();
}

void speed_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *speed_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(speed_ta, "100");
  send_gcode_speed();
}

void speed_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  speed_buttons_map_id = id;
  esp3d_log("Button %s clicked", speed_buttons_map[id]);
}

bool speed_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::speed) return false;
  esp3d_log("Speed value  %s", value);
  lv_label_set_text(label_current_speed_value, value);
  return true;
}

void speed_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Speed screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  lv_obj_t *btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_speed_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm, speed_buttons_map);
  ESP3DStyle::apply(btnm, ESP3DStyleType::buttons_matrix);
  size_t i = (sizeof(speed_buttons_map) / sizeof(speed_buttons_map[0])) - 1;
  lv_obj_set_size(btnm, ESP3D_MATRIX_BUTTON_WIDTH * i, ESP3D_MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, speed_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, speed_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Current Speed label
  lv_obj_t *label_current_speed = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_speed, LV_SYMBOL_SPEED);
  ESP3DStyle::apply(label_current_speed, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_speed, LV_ALIGN_TOP_LEFT,
               ESP3D_BUTTON_PRESSED_OUTLINE, ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_speed);

  // Current Speed value
  std::string current_speed_value_init =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::speed);

  label_current_speed_value = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_speed_value,
                    current_speed_value_init.c_str());
  ESP3DStyle::apply(label_current_speed_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_speed_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_speed_value, label_current_speed,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1, "%");
  ESP3DStyle::apply(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_speed_value,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  // Button up
  lv_obj_t *btn_up = symbolButton::create(
      ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn_up, label_current_speed_value, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  // speed input
  lv_obj_t *speed_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(speed_ta, speed_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_textarea_set_accepted_chars(speed_ta, "0123456789");
  lv_textarea_set_max_length(speed_ta, 3);
  lv_textarea_set_one_line(speed_ta, true);
  lv_textarea_set_text(speed_ta, current_speed_value_init.c_str());
  lv_obj_set_style_text_align(speed_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(speed_ta, LV_HOR_RES / 6);
  lv_obj_align_to(speed_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);

  // label
  lv_obj_t *label_ta = lv_label_create(ui_new_screen);
  lv_label_set_text(label_ta, LV_SYMBOL_SPEED);
  ESP3DStyle::apply(label_ta, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_ta, speed_ta, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2, "%");
  ESP3DStyle::apply(label_unit2, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit2, speed_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // button down
  lv_obj_t *btn_down = symbolButton::create(
      ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, speed_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, speed_btn_down_event_cb, LV_EVENT_CLICKED,
                      speed_ta);

  lv_obj_add_event_cb(btn_up, speed_btn_up_event_cb, LV_EVENT_CLICKED,
                      speed_ta);

  // Button Ok
  lv_obj_t *btn_ok =
      symbolButton::create(ui_new_screen, LV_SYMBOL_OK);
  lv_obj_align_to(btn_ok, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_ok, speed_btn_ok_event_cb, LV_EVENT_CLICKED,
                      speed_ta);

  // Button Reset
  lv_obj_t *btn_reset =
      symbolButton::create(ui_new_screen, LV_SYMBOL_GAUGE);
  lv_obj_align_to(btn_reset, btn_ok, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_reset, speed_btn_reset_event_cb, LV_EVENT_CLICKED,
                      speed_ta);

  // keypad
  lv_obj_t *speed_kb = lv_keyboard_create(ui_new_screen);
  lv_keyboard_set_mode(speed_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(speed_kb, NULL);
  lv_obj_align_to(speed_kb, speed_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(speed_kb);
  lv_obj_set_content_width(speed_kb, LV_HOR_RES - lv_obj_get_x(speed_kb) -
                                         2 * ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(speed_kb, ESP3D_BUTTON_RADIUS , LV_PART_MAIN);
  lv_obj_add_flag(speed_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(speed_ta, speed_ta_event_cb, LV_EVENT_ALL, speed_kb);

  esp3dTftui.set_current_screen(ESP3DScreenType::speed);
}
}  // namespace speedScreen