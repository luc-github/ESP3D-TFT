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

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "main_screen.h"
#include "rendering/esp3d_rendering_client.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace fanScreen {
std::string fan_value = "0";
const char *fan_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t fan_buttons_map_id = 0;
lv_obj_t *label_current_fan_value = NULL;
lv_obj_t *btnm_target = NULL;
lv_obj_t *btnback = NULL;
lv_obj_t *label_current_fan = NULL;

bool intialization_done = false;

#define HEATER_COUNT 2
#define HEATER_LABEL_SIZE 2
std::string heaters_values[HEATER_COUNT];
char *heater_buttons_map[HEATER_COUNT + 1];
bool heater_buttons_visibility_map[HEATER_COUNT] = {false, false};
int8_t heater_buttons_visibility_map_id[HEATER_COUNT]{-1, -1};

const char *heater_buttons_label_map[HEATER_COUNT] = {LV_SYMBOL_EXTRUDER "1",
                                                      LV_SYMBOL_EXTRUDER "2"};

uint8_t heater_buttons_map_id = 0;

lv_timer_t *fan_screen_delay_timer = NULL;

uint8_t get_map_size() {
  for (uint8_t i = 0; i < HEATER_COUNT + 1; i++) {
    if (strlen(heater_buttons_map[i]) == 0) {
      return i;
    }
  }
  esp3d_log("heater_buttons_map size is undefined");
  return 0;
}

const char **get_heater_buttons_map() {
  esp3d_log("get_heater_buttons_map");
  esp3d_log("heater_buttons_map size is : %d", get_map_size());
  bool update = false;
  // check if different from current
  for (int i = 0; i < HEATER_COUNT; i++) {
    bool detected = heaters_values[i] != "#";
    esp3d_log("Heater %d, value : %s detected: %s", i,
              heaters_values[i].c_str(), detected ? "true" : "false");
    if (detected != heater_buttons_visibility_map[i]) {
      esp3d_log("Update needed because detected, current %s new: %s",
                heater_buttons_visibility_map[i] ? "true" : "false ",
                detected ? "true" : "false ");
      update = true;
      heater_buttons_visibility_map[i] = detected;
    }
  }
  //  if yes update
  if (update) {
    esp3d_log("Update matrix buttons");
    heater_buttons_map_id = 0;
    uint8_t h = 0;
    for (int i = 0; i < HEATER_COUNT; i++) {
      esp3d_log("Heater %d, detected: %s", i,
                heater_buttons_visibility_map[i] ? "true" : "false");
      strcpy(heater_buttons_map[h], "");
      if (heater_buttons_visibility_map[i]) {
        esp3d_log("Add heater %d to matrix to position %d", i, h);
        strcpy(heater_buttons_map[h], heater_buttons_label_map[i]);
        heater_buttons_visibility_map[i] = true;
        heater_buttons_visibility_map_id[i] = h;
        esp3d_log("Heater button %d, label: %s", h, heater_buttons_map[h]);
        h++;
      } else {
        esp3d_log("remove heater %d to matrix ", i);
        heater_buttons_visibility_map[i] = false;
        heater_buttons_visibility_map_id[i] = -1;
      }
    }
    // last map entries must be empty
    for (int i = h; i < HEATER_COUNT; i++) {
      strcpy(heater_buttons_map[i], "");
    }
  }
  esp3d_log("heater_buttons_map size is : %d", get_map_size());
  // then return
  return (const char **)heater_buttons_map;
}

bool updateBtnMatrix() {
  // check if different from current
  //  if yes update
  // then apply style
  lv_btnmatrix_set_map(btnm_target, get_heater_buttons_map());
  apply_style(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_target);
  size_t i = get_map_size();
  lv_obj_set_size(btnm_target, BACK_BUTTON_WIDTH * i, MATRIX_BUTTON_HEIGHT);
  esp3d_log("child count: %d", i);
  // lv_obj_add_state(obj, LV_STATE_DISABLED);
  if (heater_buttons_map_id > i) heater_buttons_map_id = 0;
  lv_btnmatrix_set_btn_ctrl(btnm_target, heater_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  return true;
}

uint8_t get_heater_buttons_map_id(int8_t id) {
  esp3d_log("get_heater_buttons_map_id %d", id);
  for (uint8_t i = 0; i < HEATER_COUNT; i++) {
    if (heater_buttons_visibility_map_id[i] == id) {
      esp3d_log("give id %d", i);
      return i;
    }
  }
  esp3d_log("get_heater_buttons_map_id %d not found", id);
  return 0;
}

int8_t get_heater_buttons_map_button_id(uint8_t id) {
  if (id >= HEATER_COUNT) {
    esp3d_log_e("get_heater_buttons_map_button_id %d not found", id);
    return -1;
  }
  esp3d_log("get_heater_buttons_map_button_id %d, give id %d", id,
            heater_buttons_visibility_map_id[id]);
  return heater_buttons_visibility_map_id[id];
}

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
    if (fan_screen_delay_timer) return;
    fan_screen_delay_timer = lv_timer_create(fan_screen_delay_timer_cb,
                                             BUTTON_ANIMATION_DELAY, NULL);
  } else {
    fan_screen_delay_timer_cb(NULL);
  }
}

void send_gcode_fan() {
  std::string fan_value_str = "M106 S";
  fan_value_str += fan_value;
  renderingClient.sendGcode(fan_value_str.c_str());
}

void fan_ta_event_cb(lv_event_t *e) {
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
    fan_value = lv_textarea_get_text(ta);
    if (std::atoi(fan_value.c_str()) > 100) {
      lv_textarea_set_text(ta, "100");
    }
    if (std::atoi(fan_value.c_str()) < 0) {
      lv_textarea_set_text(ta, "0");
    }
    if (fan_value.length() == 0) {
      lv_textarea_set_text(ta, "1");
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    fan_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", fan_value.c_str());
  }
}

void fan_btn_up_event_cb(lv_event_t *e) {
  esp3d_log("Up");
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan);
  int step = atoi(fan_buttons_map[fan_buttons_map_id]);
  esp3d_log("Step: %d, Speed: %d", step, fan_int);
  fan_int += step;
  if (fan_int > 100) fan_int = 100;
  esp3d_log("new Speed: %d", fan_int);
  fan = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan.c_str());
  send_gcode_fan();
}

void fan_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan);
  int step = atoi(fan_buttons_map[fan_buttons_map_id]);
  fan_int -= step;
  if (fan_int < 0) fan_int = 0;
  fan = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan.c_str());
  send_gcode_fan();
}

void fan_btn_ok_event_cb(lv_event_t *e) {
  esp3d_log("Ok clicked");
  send_gcode_fan();
}

void fan_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(fan_ta, "0");
  send_gcode_fan();
}

void fan_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  fan_buttons_map_id = id;
  esp3d_log("Button %s clicked", fan_buttons_map[id]);
}

void heater_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    heater_buttons_map_id = id;
    lv_label_set_text(label_current_fan,
                      heater_buttons_map[heater_buttons_map_id]);
    /*  lv_label_set_text(label_current_temperature,
                       heater_buttons_map[heater_buttons_map_id]);
     lv_label_set_text(label_target_temperature,
                       heater_buttons_map[heater_buttons_map_id]);
     uint8_t target = get_heater_buttons_map_id(id);
     std::string temp_value;
     std::string temperatures_value_init;
     switch (target) {
       case 0:  // extruder 0
         temp_value = esp3dTftValues.get_string_value(
             ESP3DValuesIndex::ext_0_temperature);
         if (temp_value == "?") temp_value = "0";
         lv_label_set_text(label_current_temperature_value, temp_value.c_str());
         lv_label_set_text(label_target_temperature_value,
                           esp3dTftValues.get_string_value(
                               ESP3DValuesIndex::ext_0_target_temperature));

         temperatures_value_init = std::to_string(
             std::atoi(temp_value.c_str()) > 0 ? std::atoi(temp_value.c_str())
                                               : 0);
         temperatures_value = temperatures_value_init;
         lv_textarea_set_text(temperatures_ta, temperatures_value_init.c_str());
         break;
       case 1:  // extruder 1
         temp_value = esp3dTftValues.get_string_value(
             ESP3DValuesIndex::ext_1_temperature);
         if (temp_value == "?") temp_value = "0";
         lv_label_set_text(label_current_temperature_value, temp_value.c_str());
         lv_label_set_text(label_target_temperature_value,
                           esp3dTftValues.get_string_value(
                               ESP3DValuesIndex::ext_1_target_temperature));
         temperatures_value_init = std::to_string(
             std::atoi(temp_value.c_str()) > 0 ? std::atoi(temp_value.c_str())
                                               : 0);
         temperatures_value = temperatures_value_init;
         lv_textarea_set_text(temperatures_ta, temperatures_value_init.c_str());
         break;
       case 2:  // bed
         temp_value =
             esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_temperature);
         if (temp_value == "?") temp_value = "0";
         lv_label_set_text(label_current_temperature_value, temp_value.c_str());
         lv_label_set_text(label_target_temperature_value,
                           esp3dTftValues.get_string_value(
                               ESP3DValuesIndex::bed_target_temperature));
         temperatures_value_init = std::to_string(
             std::atoi(temp_value.c_str()) > 0 ? std::atoi(temp_value.c_str())
                                               : 0);
         temperatures_value = temperatures_value_init;
         lv_textarea_set_text(temperatures_ta, temperatures_value_init.c_str());
         break;
       default:
         break;
     };*/
    esp3d_log("Button %s clicked", heater_buttons_map[id]);
  }
}

bool fan_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action) {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::fan) return false;
  esp3d_log("Speed value  %s", value);
  lv_label_set_text(label_current_fan_value, value);
  return true;
}

void fan_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Fan screen creation");
  if (!intialization_done) {
    esp3d_log("fan screen initialization");
    // by default all heater are detected
    //  update will occur only if different
    for (uint8_t i = 0; i < HEATER_COUNT; i++) {
      heaters_values[i] = "?";
      // this one must be called only once or need to free memory
      heater_buttons_map[i] =
          (char *)calloc(HEATER_LABEL_SIZE + 1, sizeof(char));
      strcpy(heater_buttons_map[i], heater_buttons_label_map[i]);
      esp3d_log("Heater %d, value : %s, label: %s", i,
                heaters_values[i].c_str(), heater_buttons_map[i]);
    }
    // the real max final is 0
    heater_buttons_map[HEATER_COUNT] = (char *)calloc(1, sizeof(char));

    intialization_done = true;
    // last map entry must be empty
    strcpy(heater_buttons_map[HEATER_COUNT], "");
    get_heater_buttons_map();
  }

  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_fan_back_handler, LV_EVENT_CLICKED,
                      NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm, fan_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, fan_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, fan_matrix_buttons_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);

  // Current Fan label
  label_current_fan = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_fan,
                    heater_buttons_map[heater_buttons_map_id]);
  apply_style(label_current_fan, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_fan, LV_ALIGN_TOP_LEFT,
               CURRENT_BUTTON_PRESSED_OUTLINE, CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_fan);

  // Current Fan value
  std::string current_fan_value_init =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan);

  label_current_fan_value = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_fan_value, current_fan_value_init.c_str());
  apply_style(label_current_fan_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_fan_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_fan_value, label_current_fan,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1, "%");
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_fan_value, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Button up
  lv_obj_t *btn_up = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn_up, label_current_fan_value, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  // fan input
  lv_obj_t *fan_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(fan_ta, fan_ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_textarea_set_accepted_chars(fan_ta, "0123456789");
  lv_textarea_set_max_length(fan_ta, 3);
  lv_textarea_set_one_line(fan_ta, true);
  lv_textarea_set_text(fan_ta, current_fan_value_init.c_str());
  lv_obj_set_style_text_align(fan_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(fan_ta, LV_HOR_RES / 6);
  lv_obj_align_to(fan_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  // label
  lv_obj_t *label_ta = lv_label_create(ui_new_screen);
  lv_label_set_text(label_ta, LV_SYMBOL_FAN);
  apply_style(label_ta, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_ta, fan_ta, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2, "%");
  apply_style(label_unit2, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit2, fan_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // button down
  lv_obj_t *btn_down = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, fan_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, fan_btn_down_event_cb, LV_EVENT_CLICKED,
                      fan_ta);

  lv_obj_add_event_cb(btn_up, fan_btn_up_event_cb, LV_EVENT_CLICKED, fan_ta);

  // Button Ok
  lv_obj_t *btn_ok =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_OK);
  lv_obj_align_to(btn_ok, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_ok, fan_btn_ok_event_cb, LV_EVENT_CLICKED, fan_ta);

  // Button Reset
  lv_obj_t *btn_reset =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_POWER);
  lv_obj_align_to(btn_reset, btn_ok, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_reset, fan_btn_reset_event_cb, LV_EVENT_CLICKED,
                      fan_ta);

  // keypad
  lv_obj_t *fan_kb = lv_keyboard_create(ui_new_screen);
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

  // Target selector button matrix
  btnm_target = lv_btnmatrix_create(ui_new_screen);
  // build heater buttons map
  updateBtnMatrix();
  lv_obj_add_event_cb(btnm_target, heater_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, fan_ta);

  esp3dTftui.set_current_screen(ESP3DScreenType::fan);
}
}  // namespace fanScreen