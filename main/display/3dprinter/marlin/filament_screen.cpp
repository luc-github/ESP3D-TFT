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
void filament_screen();

std::string filament_value = "100";
const char *filament_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t filament_buttons_map_id = 0;

lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back,
                                ESP3DStyleType style);
lv_obj_t *create_symbol_button(lv_obj_t *container, const char *text,
                               int width = SYMBOL_BUTTON_WIDTH,
                               int height = SYMBOL_BUTTON_HEIGHT,
                               bool center = true, bool slash = false,
                               int rotation = 0);

lv_timer_t *filament_screen_delay_timer = NULL;

void filament_screen_delay_timer_cb(lv_timer_t *timer) {
  if (filament_screen_delay_timer) {
    lv_timer_del(filament_screen_delay_timer);
    filament_screen_delay_timer = NULL;
  }
  main_screen();
}

void event_button_filament_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    filament_screen_delay_timer = lv_timer_create(
        filament_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    filament_screen_delay_timer_cb(NULL);
  }
}

void filament_ta_event_cb(lv_event_t *e) {
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
    filament_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    // No idea how to change ta focus to another control
    // every tests I did failed
    // so I refresh all screen ... orz
    filament_value = lv_textarea_get_text(ta);
    filament_screen();
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    filament_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", filament_value.c_str());
  }
}

void filament_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *filament_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string filament_value = lv_textarea_get_text(filament_ta);
  int filament_int = std::stoi(filament_value);
  int step = atoi(filament_buttons_map[filament_buttons_map_id]);
  filament_int += step;
  filament_value = std::to_string(filament_int);
  lv_textarea_set_text(filament_ta, filament_value.c_str());
}

void filament_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *filament_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string filament_value = lv_textarea_get_text(filament_ta);
  int filament_int = std::stoi(filament_value);
  int step = atoi(filament_buttons_map[filament_buttons_map_id]);
  filament_int -= step;
  filament_value = std::to_string(filament_int);
  lv_textarea_set_text(filament_ta, filament_value.c_str());
}

void filament_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *filament_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string filament_value = lv_textarea_get_text(filament_ta);
  esp3d_log("Ok: %s", filament_value.c_str());
}

void filament_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *filament_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(filament_ta, "100");
}

void filament_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  filament_buttons_map_id = id;
  esp3d_log("Button %s clicked", filament_buttons_map[id]);
}

void filament_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Filament screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_filament_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_t *ui_main_container = create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::simple_container);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_main_container);
  lv_btnmatrix_set_map(btnm, filament_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, filament_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, filament_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *label = lv_label_create(ui_main_container);
  lv_label_set_text(label, LV_SYMBOL_FILAMENT);
  apply_style(label, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label);
  int32_t label_width = lv_obj_get_width(label);
  int32_t label_height = lv_obj_get_height(label);
  lv_obj_set_style_transform_pivot_x(label, label_width / 2, 0);
  lv_obj_set_style_transform_pivot_y(label, label_height / 2, 0);
  lv_obj_set_style_transform_angle(label, 900, 0);

  lv_obj_set_y(label, lv_obj_get_height(ui_main_container) / 2 -
                          lv_obj_get_height(label) / 2);

  lv_obj_t *filament_ta = lv_textarea_create(ui_main_container);
  lv_obj_add_event_cb(filament_ta, filament_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_textarea_set_accepted_chars(filament_ta, "0123456789");
  lv_textarea_set_max_length(filament_ta, 3);
  lv_textarea_set_one_line(filament_ta, true);
  esp3d_log("value: %s", filament_value.c_str());
  std::string filament_value_init = filament_value;
  lv_textarea_set_text(filament_ta, filament_value_init.c_str());
  lv_obj_set_style_text_align(filament_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(filament_ta, LV_HOR_RES / 6);

  lv_obj_align_to(filament_ta, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  lv_obj_t *btn =
      create_symbol_button(ui_main_container, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn, filament_ta, LV_ALIGN_OUT_TOP_MID, 0,
                  -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, filament_btn_up_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  btn = create_symbol_button(ui_main_container,
                             LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn, filament_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn, filament_btn_down_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  label = lv_label_create(ui_main_container);
  lv_label_set_text(label, "mm");
  apply_style(label, ESP3DStyleType::bg_label);

  lv_obj_align_to(label, filament_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  btn = create_symbol_button(ui_main_container, LV_SYMBOL_OK);
  lv_obj_align_to(btn, label, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn, filament_btn_ok_event_cb, LV_EVENT_CLICKED,
                      filament_ta);
  lv_obj_t *btn2 = create_symbol_button(ui_main_container, LV_SYMBOL_GAUGE);
  lv_obj_align_to(btn2, btn, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn2, filament_btn_reset_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  lv_obj_t *filament_kb = lv_keyboard_create(ui_main_container);
  lv_keyboard_set_mode(filament_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(filament_kb, NULL);
  lv_obj_align_to(filament_kb, filament_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(filament_kb);
  lv_obj_set_content_width(filament_kb, LV_HOR_RES - lv_obj_get_x(filament_kb) -
                                            2 * CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(filament_kb, CURRENT_BUTTON_RADIUS_VALUE,
                          LV_PART_MAIN);
  lv_obj_add_flag(filament_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(filament_ta, filament_ta_event_cb, LV_EVENT_ALL,
                      filament_kb);
  esp3dTftui.set_current_screen(ESP3DScreenType::filament);
}