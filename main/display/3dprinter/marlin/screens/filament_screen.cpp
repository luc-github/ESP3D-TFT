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

#include "filament_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "menu_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "temperatures_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace filamentScreen {
std::string filament_value = "0";
const char *filament_distance_steps_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t filament_distance_steps_buttons_map_id = 0;
lv_obj_t *label_current_temperature_filament_value = NULL;
lv_obj_t *btnm_target = NULL;
lv_obj_t *btnback = NULL;
lv_obj_t *label_current_temperature_filament = NULL;

bool intialization_done = false;

#define FILAMENT_COUNT 2
#define FILAMENT_LABEL_SIZE 2
uint8_t nb_filaments = FILAMENT_COUNT;
char *filament_buttons_map[FILAMENT_COUNT + 1];
uint8_t filament_buttons_map_id = 0;
const char *filament_buttons_label_map[FILAMENT_COUNT] = {
    LV_SYMBOL_EXTRUDER "1", LV_SYMBOL_EXTRUDER "2"};

lv_timer_t *filament_screen_delay_timer = NULL;

uint8_t get_map_size() {
  for (uint8_t i = 0; i < FILAMENT_COUNT + 1; i++) {
    if (strlen(filament_buttons_map[i]) == 0) {
      return i;
    }
  }
  esp3d_log("filament_buttons_map size is undefined");
  return 0;
}

const char **get_filament_buttons_map() {
  esp3d_log("get_filament_buttons_map");
  esp3d_log("filament_buttons_map size is : %d", get_map_size());

  //  if yes update
  if (get_map_size() != nb_filaments) {
    esp3d_log("Update matrix buttons");
    // update matrix buttons
    if (nb_filaments == 1) {
      strcpy(filament_buttons_map[1], "");
    } else {
      strcpy(filament_buttons_map[1], filament_buttons_label_map[1]);
    }
  }
  esp3d_log("filament_buttons_map size is : %d", get_map_size());
  // then return
  return (const char **)filament_buttons_map;
}

bool updateBtnMatrix() {
  // check if different from current
  //  if yes update
  // then apply style
  lv_btnmatrix_set_map(btnm_target, get_filament_buttons_map());
  apply_style(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_target);
  size_t i = get_map_size();
  lv_obj_set_size(btnm_target, BACK_BUTTON_WIDTH * i, MATRIX_BUTTON_HEIGHT);
  esp3d_log("child count: %d", i);
  // lv_obj_add_state(obj, LV_STATE_DISABLED);
  if (filament_buttons_map_id > i) filament_buttons_map_id = 0;
  lv_btnmatrix_set_btn_ctrl(btnm_target, filament_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  return true;
}

void filament_screen_delay_timer_cb(lv_timer_t *timer) {
  if (filament_screen_delay_timer) {
    lv_timer_del(filament_screen_delay_timer);
    filament_screen_delay_timer = NULL;
  }
  menuScreen::menu_screen();
}

void filament_screen_edit_delay_timer_cb(lv_timer_t *timer) {
  if (filament_screen_delay_timer) {
    lv_timer_del(filament_screen_delay_timer);
    filament_screen_delay_timer = NULL;
  }
  temperaturesScreen::temperatures_screen(filament_buttons_map_id,
                                          ESP3DScreenType::filament);
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

void event_button_filament_edit_handler(lv_event_t *e) {
  esp3d_log("edit Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (filament_screen_delay_timer) return;
    filament_screen_delay_timer = lv_timer_create(
        filament_screen_edit_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    filament_screen_edit_delay_timer_cb(NULL);
  }
}

void send_gcode_filament(const char *sign = "") {
  std::string filament_value_str = "G91";
  renderingClient.sendGcode(filament_value_str.c_str());

  if (filament_buttons_map_id == 0) {
    filament_value_str = "T0";
  } else {
    filament_value_str = "T1";
  }
  renderingClient.sendGcode(filament_value_str.c_str());

  filament_value_str = "G1 E";
  filament_value_str += sign;
  filament_value_str += filament_value;
  renderingClient.sendGcode(filament_value_str.c_str());

  filament_value_str = "G90";
  renderingClient.sendGcode(filament_value_str.c_str());
}

void filament_ta_event_cb(lv_event_t *e) {
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
    filament_value = lv_textarea_get_text(ta);

    if (filament_value.length() == 0) {
      lv_textarea_set_text(ta, "0");
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    filament_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", filament_value.c_str());
  }
}

void filament_btn_up_event_cb(lv_event_t *e) {
  esp3d_log("Up");
  send_gcode_filament();
}

void filament_btn_down_event_cb(lv_event_t *e) { send_gcode_filament("-"); }

void filament_distance_steps_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  filament_distance_steps_buttons_map_id = id;
  esp3d_log("Button %s clicked", filament_distance_steps_buttons_map[id]);
  lv_textarea_set_text(ta, filament_distance_steps_buttons_map[id]);
}

void filament_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  filament_buttons_map_id = lv_btnmatrix_get_selected_btn(obj);
  std::string current_temperature_filament_value_init =
      filament_buttons_map_id == 0
          ? esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature)
          : esp3dTftValues.get_string_value(
                ESP3DValuesIndex::ext_1_temperature);
  lv_label_set_text(label_current_temperature_filament,
                    filament_buttons_map[filament_buttons_map_id]);
  lv_label_set_text(label_current_temperature_filament_value,
                    current_temperature_filament_value_init.c_str());
  esp3d_log("Button %s clicked", filament_buttons_map[filament_buttons_map_id]);
}

bool filament_value_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action) {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::filament)
    return false;
  if (index == ESP3DValuesIndex::ext_1_temperature ||
      index == ESP3DValuesIndex::ext_1_target_temperature) {
    if (index == ESP3DValuesIndex::ext_1_temperature &&
        filament_distance_steps_buttons_map_id == 1) {
      esp3d_log("Update temperature value %s", value);
      lv_label_set_text(label_current_temperature_filament_value, value);
    }
    esp3d_log("Check if extruder 1 data %s:", value);
    uint nb_filaments_tmp = 2;
    if (strcmp(value, "#") == 0) {
      esp3d_log("No extruder 1, only one filament");
      nb_filaments_tmp = 1;
    }
    if (nb_filaments_tmp != nb_filaments) {
      esp3d_log("Update nb of filaments");
      nb_filaments = nb_filaments_tmp;
    } else {
      // no update needed
      esp3d_log("No update needed");
      return false;
    }
    // update needed for filament buttons
    updateBtnMatrix();
    return true;
  }

  if (index == ESP3DValuesIndex::ext_0_temperature &&
      filament_distance_steps_buttons_map_id == 0) {
    esp3d_log("Update filament value %s", value);
    lv_label_set_text(label_current_temperature_filament_value, value);
    return true;
  }

  return false;
}

void filament_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Filament screen creation");
  if (!intialization_done) {
    esp3d_log("filament screen initialization");
    // by default all filaments are detected
    //  update will occur only if different
    for (uint8_t i = 0; i < FILAMENT_COUNT; i++) {
      // this one must be called only once or need to free memory
      filament_buttons_map[i] =
          (char *)calloc(FILAMENT_LABEL_SIZE + 1, sizeof(char));
      strcpy(filament_buttons_map[i], filament_buttons_label_map[i]);
    }
    // the real max final is 0
    filament_buttons_map[FILAMENT_COUNT] = (char *)calloc(1, sizeof(char));
    // last map entry must be empty
    strcpy(filament_buttons_map[FILAMENT_COUNT], "");
    get_filament_buttons_map();
    intialization_done = true;
  }

  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_filament_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm, filament_distance_steps_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, filament_distance_steps_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);

  // Text area label
  lv_obj_t *label_ta = lv_label_create(ui_new_screen);
  lv_label_set_text(label_ta, LV_SYMBOL_FILAMENT);
  apply_style(label_ta, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_ta);
  int32_t label_width = lv_obj_get_width(label_ta);
  int32_t label_height = lv_obj_get_height(label_ta);
  lv_obj_set_style_transform_pivot_x(label_ta, label_width / 2, 0);
  lv_obj_set_style_transform_pivot_y(label_ta, label_height / 2, 0);
  lv_obj_set_style_transform_angle(label_ta, 900, 0);
  lv_obj_update_layout(label_ta);

  size_t x = lv_obj_get_width(label_ta) + (2 * CURRENT_BUTTON_PRESSED_OUTLINE);
  size_t y = MATRIX_BUTTON_HEIGHT + CURRENT_BUTTON_PRESSED_OUTLINE;

  // Button up
  lv_obj_t *btn_up = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_set_pos(btn_up, x, y);
  // filament input
  lv_obj_t *filament_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(filament_ta, filament_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_textarea_set_accepted_chars(filament_ta, "0123456789-.");
  lv_textarea_set_max_length(filament_ta, 3);
  lv_textarea_set_one_line(filament_ta, true);

  lv_textarea_set_text(filament_ta,
                       filament_distance_steps_buttons_map
                           [filament_distance_steps_buttons_map_id]);
  lv_obj_set_style_text_align(filament_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(filament_ta, LV_HOR_RES / 6);
  lv_obj_align_to(filament_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btnm, filament_distance_steps_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, filament_ta);

  // Text area label
  lv_obj_align_to(label_ta, filament_ta, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  apply_style(label_unit2, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit2, filament_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Current filament label
  label_current_temperature_filament = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_temperature_filament,
                    filament_buttons_map[filament_buttons_map_id]);
  apply_style(label_current_temperature_filament, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_current_temperature_filament, label_unit2,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE * 3,
                  0);
  lv_obj_update_layout(label_current_temperature_filament);

  // Current filament temperature value
  std::string current_temperature_filament_value_init =
      filament_buttons_map_id == 0
          ? esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature)
          : esp3dTftValues.get_string_value(
                ESP3DValuesIndex::ext_1_temperature);

  label_current_temperature_filament_value = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_temperature_filament_value,
                    current_temperature_filament_value_init.c_str());
  apply_style(label_current_temperature_filament_value,
              ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_temperature_filament_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_temperature_filament_value,
                  label_current_temperature_filament, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::celsius));
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_temperature_filament_value,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  lv_obj_t *btn_edit =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_EDIT);
  lv_obj_align_to(btn_edit, label_unit1, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_add_event_cb(btn_edit, event_button_filament_edit_handler,
                      LV_EVENT_CLICKED, NULL);

  // button down
  lv_obj_t *btn_down = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, filament_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, filament_btn_down_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  lv_obj_add_event_cb(btn_up, filament_btn_up_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  // keypad
  lv_obj_t *filament_kb = lv_keyboard_create(ui_new_screen);
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

  // Target selector button matrix
  btnm_target = lv_btnmatrix_create(ui_new_screen);
  // build filament buttons map
  updateBtnMatrix();
  lv_obj_add_event_cb(btnm_target, filament_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, filament_ta);

  esp3dTftui.set_current_screen(ESP3DScreenType::filament);
}
}  // namespace filamentScreen