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
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

void menu_screen();
void filament_screen(uint8_t target = 0);
std::string distance_value = "1";

const char *filament_buttons_map[] = {"1", "10", "50", "100", ""};
uint8_t filament_buttons_map_id = 0;

const char *extruder_buttons_map[] = {LV_SYMBOL_EXTRUDER "1",
                                      LV_SYMBOL_EXTRUDER "2", ""};

uint8_t extruder_buttons_map_id = 0;

lv_obj_t *label_current_distance_value = NULL;
lv_obj_t *label_current_filament = NULL;

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
  menu_screen();
}

void event_button_filament_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (filament_screen_delay_timer) return;
    filament_screen_delay_timer = lv_timer_create(
        filament_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    filament_screen_delay_timer_cb(NULL);
  }
}

void distance_ta_event_cb(lv_event_t *e) {
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
    distance_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    // No idea how to change ta focus to another control
    // every tests I did failed
    // so I refresh all screen ... orz
    distance_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", distance_value.c_str());
    filament_screen(extruder_buttons_map_id);
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    distance_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", distance_value.c_str());
  }
}

void filament_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string distance_value = lv_textarea_get_text(distance_ta);
  esp3d_log("Extract: %s mm", distance_value.c_str());
}

void filament_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string distance_value = lv_textarea_get_text(distance_ta);
  esp3d_log("Revert: %s mm", distance_value.c_str());
}

void filament_btn_100_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_textarea_set_text(distance_ta, "100");
  esp3d_log("Set distance to 100");
}

void filament_btn_50_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_textarea_set_text(distance_ta, "50");
  esp3d_log("Set distance to 50");
}

void filament_btn_10_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_textarea_set_text(distance_ta, "10");
  esp3d_log("Set distance to 10");
}

void filament_btn_1_event_cb(lv_event_t *e) {
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_textarea_set_text(distance_ta, "1");
  esp3d_log("Set distance to 1");
}

void distance_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *distance_ta = (lv_obj_t *)lv_event_get_user_data(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  filament_buttons_map_id = id;
  lv_textarea_set_text(distance_ta,
                       filament_buttons_map[filament_buttons_map_id]);
  esp3d_log("Button %s clicked", filament_buttons_map[id]);
}

void extruder_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  extruder_buttons_map_id = id;
  lv_label_set_text(label_current_filament,
                    extruder_buttons_map[extruder_buttons_map_id]);
  esp3d_log("Button %s clicked", extruder_buttons_map[id]);
}

void filament_screen(uint8_t target) {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  extruder_buttons_map_id = target;
  // Screen creation
  esp3d_log("filament screen creation for target %d", target);
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);
  // back button
  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_filament_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button bar
  // 100 mm
  lv_obj_t *btn_100 = create_symbol_button(
      ui_new_screen, "100", MATRIX_BUTTON_HEIGHT, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btn_100, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  // 50 mm
  lv_obj_t *btn_50 = create_symbol_button(
      ui_new_screen, "50", MATRIX_BUTTON_HEIGHT, MATRIX_BUTTON_HEIGHT);
  lv_obj_align_to(btn_50, btn_100, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  // 10 mm
  lv_obj_t *btn_10 = create_symbol_button(
      ui_new_screen, "10", MATRIX_BUTTON_HEIGHT, MATRIX_BUTTON_HEIGHT);
  lv_obj_align_to(btn_10, btn_50, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  // 1 mm
  lv_obj_t *btn_1 = create_symbol_button(
      ui_new_screen, "1", MATRIX_BUTTON_HEIGHT, MATRIX_BUTTON_HEIGHT);
  lv_obj_align_to(btn_1, btn_10, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Target selector button matrix
  lv_obj_t *btnm_target = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm_target, extruder_buttons_map);
  apply_style(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_set_height(btnm_target, MATRIX_BUTTON_HEIGHT);
  lv_btnmatrix_set_btn_ctrl(btnm_target, extruder_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm_target, extruder_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  // Label current axis
  label_current_filament = lv_label_create(ui_new_screen);
  lv_label_set_text(
      label_current_filament,
      extruder_buttons_map[extruder_buttons_map_id]);  // need to change
                                                       // according axis
  apply_style(label_current_filament, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_filament, LV_ALIGN_TOP_LEFT,
               CURRENT_BUTTON_PRESSED_OUTLINE, CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_filament);

  // Label current axis e
  label_current_distance_value = lv_label_create(ui_new_screen);

  std::string current_distance_value_init = "280.00";

  lv_label_set_text(label_current_distance_value,
                    current_distance_value_init.c_str());

  apply_style(label_current_distance_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_distance_value, LV_HOR_RES / 5);
  lv_obj_align_to(label_current_distance_value, label_current_filament,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::celcius));
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_distance_value,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // Button up
  lv_obj_t *btn_up =
      create_symbol_button(ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn_up, label_current_distance_value, LV_ALIGN_OUT_BOTTOM_MID,
                  0, CURRENT_BUTTON_PRESSED_OUTLINE);
  // Text area
  lv_obj_t *distance_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(distance_ta, distance_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_textarea_set_accepted_chars(distance_ta, "0123456789");
  lv_textarea_set_max_length(distance_ta, 7);
  lv_textarea_set_one_line(distance_ta, true);
  esp3d_log("value: %s", distance_value.c_str());
  std::string distance_value_init = distance_value;
  lv_textarea_set_text(distance_ta, distance_value_init.c_str());
  lv_obj_set_style_text_align(distance_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(distance_ta, LV_HOR_RES / 5);

  lv_obj_align_to(distance_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btn_up, filament_btn_up_event_cb, LV_EVENT_CLICKED,
                      distance_ta);

  lv_obj_add_event_cb(btn_100, filament_btn_100_event_cb, LV_EVENT_CLICKED,
                      distance_ta);
  lv_obj_add_event_cb(btn_50, filament_btn_50_event_cb, LV_EVENT_CLICKED,
                      distance_ta);
  lv_obj_add_event_cb(btn_10, filament_btn_10_event_cb, LV_EVENT_CLICKED,
                      distance_ta);
  lv_obj_add_event_cb(btn_1, filament_btn_1_event_cb, LV_EVENT_CLICKED,
                      distance_ta);

  // Label target axis
  lv_obj_t *label_target = lv_label_create(ui_new_screen);
  lv_label_set_text(label_target, LV_SYMBOL_FILAMENT);
  apply_style(label_target, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_target);
  int32_t label_width = lv_obj_get_width(label_target);
  int32_t label_height = lv_obj_get_height(label_target);
  lv_obj_set_style_transform_pivot_x(label_target, label_width / 2, 0);
  lv_obj_set_style_transform_pivot_y(label_target, label_height / 2, 0);
  lv_obj_set_style_transform_angle(label_target, 900, 0);
  lv_obj_align_to(label_target, distance_ta, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  apply_style(label_unit2, ESP3DStyleType::bg_label);

  lv_obj_align_to(label_unit2, distance_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Keyboard
  lv_obj_t *filament_kb = lv_keyboard_create(ui_new_screen);
  lv_keyboard_set_mode(filament_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(filament_kb, NULL);
  lv_obj_update_layout(label_unit2);
  lv_obj_set_content_width(filament_kb,
                           LV_HOR_RES - (lv_obj_get_x(label_unit2) +
                                         CURRENT_BUTTON_PRESSED_OUTLINE));
  lv_obj_align_to(filament_kb, distance_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_set_style_radius(filament_kb, CURRENT_BUTTON_RADIUS_VALUE,
                          LV_PART_MAIN);
  lv_obj_add_flag(filament_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(distance_ta, distance_ta_event_cb, LV_EVENT_ALL,
                      filament_kb);
  // Button down
  lv_obj_t *btn_down =
      create_symbol_button(ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, distance_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, filament_btn_down_event_cb, LV_EVENT_CLICKED,
                      distance_ta);

  esp3dTftui.set_current_screen(ESP3DScreenType::filament);
}