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

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

void main_screen();
void positions_screen();

std::string positions_value = "100";
const char *positions_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t positions_buttons_map_id = 0;

lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back,
                                ESP3DStyleType style);
lv_obj_t *create_symbol_button(lv_obj_t *container, const char *text,
                               int width = SYMBOL_BUTTON_WIDTH,
                               int height = SYMBOL_BUTTON_HEIGHT,
                               bool center = true, bool slash = false,
                               int rotation = 0);

lv_timer_t *positions_screen_delay_timer = NULL;

void positions_screen_delay_timer_cb(lv_timer_t *timer) {
  if (positions_screen_delay_timer) {
    lv_timer_del(positions_screen_delay_timer);
    positions_screen_delay_timer = NULL;
  }
  main_screen();
}

void event_button_positions_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (positions_screen_delay_timer) return;
    positions_screen_delay_timer = lv_timer_create(
        positions_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    positions_screen_delay_timer_cb(NULL);
  }
}

void positions_ta_event_cb(lv_event_t *e) {
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
    positions_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    // No idea how to change ta focus to another control
    // every tests I did failed
    // so I refresh all screen ... orz
    positions_value = lv_textarea_get_text(ta);
    positions_screen();
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    positions_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", positions_value.c_str());
  }
}

void positions_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *positions_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string positions_value = lv_textarea_get_text(positions_ta);
  int positions_int = std::stoi(positions_value);
  int step = atoi(positions_buttons_map[positions_buttons_map_id]);
  positions_int += step;
  positions_value = std::to_string(positions_int);
  lv_textarea_set_text(positions_ta, positions_value.c_str());
}

void positions_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *positions_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string positions_value = lv_textarea_get_text(positions_ta);
  int positions_int = std::stoi(positions_value);
  int step = atoi(positions_buttons_map[positions_buttons_map_id]);
  positions_int -= step;
  positions_value = std::to_string(positions_int);
  lv_textarea_set_text(positions_ta, positions_value.c_str());
}

void positions_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *positions_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string positions_value = lv_textarea_get_text(positions_ta);
  esp3d_log("Ok: %s", positions_value.c_str());
}

void positions_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *positions_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(positions_ta, "100");
}

void positions_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  positions_buttons_map_id = id;
  esp3d_log("Button %s clicked", positions_buttons_map[id]);
}

void positions_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Positions screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_positions_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_t *ui_main_container = create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::simple_container);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_main_container);
  lv_btnmatrix_set_map(btnm, positions_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, positions_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, positions_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *label = lv_label_create(ui_main_container);
  lv_label_set_text(label, LV_SYMBOL_JOG);
  apply_style(label, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label);
  lv_obj_set_y(label, lv_obj_get_height(ui_main_container) / 2 -
                          lv_obj_get_height(label) / 2);

  lv_obj_t *positions_ta = lv_textarea_create(ui_main_container);
  lv_obj_add_event_cb(positions_ta, positions_ta_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);
  lv_textarea_set_accepted_chars(positions_ta, "0123456789");
  lv_textarea_set_max_length(positions_ta, 3);
  lv_textarea_set_one_line(positions_ta, true);
  esp3d_log("value: %s", positions_value.c_str());
  std::string positions_value_init = positions_value;
  lv_textarea_set_text(positions_ta, positions_value_init.c_str());
  lv_obj_set_style_text_align(positions_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(positions_ta, LV_HOR_RES / 6);

  lv_obj_align_to(positions_ta, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  lv_obj_t *btn =
      create_symbol_button(ui_main_container, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn, positions_ta, LV_ALIGN_OUT_TOP_MID, 0,
                  -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, positions_btn_up_event_cb, LV_EVENT_CLICKED,
                      positions_ta);

  btn = create_symbol_button(ui_main_container,
                             LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn, positions_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, positions_btn_down_event_cb, LV_EVENT_CLICKED,
                      positions_ta);

  label = lv_label_create(ui_main_container);
  lv_label_set_text(label, "mm");
  apply_style(label, ESP3DStyleType::bg_label);

  lv_obj_align_to(label, positions_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  btn = create_symbol_button(ui_main_container, LV_SYMBOL_OK);
  lv_obj_align_to(btn, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn, positions_btn_ok_event_cb, LV_EVENT_CLICKED,
                      positions_ta);
  lv_obj_t *btn2 = create_symbol_button(ui_main_container, LV_SYMBOL_GAUGE);
  lv_obj_align_to(btn2, btn, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn2, positions_btn_reset_event_cb, LV_EVENT_CLICKED,
                      positions_ta);

  lv_obj_t *positions_kb = lv_keyboard_create(ui_main_container);
  lv_keyboard_set_mode(positions_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(positions_kb, NULL);
  lv_obj_align_to(positions_kb, positions_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(positions_kb);
  lv_obj_set_content_width(positions_kb,
                           LV_HOR_RES - lv_obj_get_x(positions_kb) -
                               2 * CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(positions_kb, CURRENT_BUTTON_RADIUS_VALUE,
                          LV_PART_MAIN);
  lv_obj_add_flag(positions_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(positions_ta, positions_ta_event_cb, LV_EVENT_ALL,
                      positions_kb);
  esp3dTftui.set_current_screen(ESP3DScreenType::positions);
}