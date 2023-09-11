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

#include "temperatures_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "filament_screen.h"
#include "main_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace temperaturesScreen {
std::string temperatures_value = "0";
const char *temperatures_buttons_map[] = {"1", "10", "50", "100", ""};
uint8_t temperatures_buttons_map_id = 0;
ESP3DScreenType screen_return = ESP3DScreenType::main;

lv_obj_t *btnm_target = NULL;

bool intialization_done = false;
#define HEATER_COUNT 3
#define HEATER_LABEL_SIZE 2
std::string heaters_values[HEATER_COUNT];
char *heater_buttons_map[HEATER_COUNT + 1];
bool heater_buttons_visibility_map[HEATER_COUNT] = {false, false, false};
int8_t heater_buttons_visibility_map_id[HEATER_COUNT]{-1, -1, -1};

const char *heater_buttons_label_map[HEATER_COUNT] = {
    LV_SYMBOL_EXTRUDER "1", LV_SYMBOL_EXTRUDER "2", LV_SYMBOL_HEAT_BED};

uint8_t heater_buttons_map_id = 0;

lv_obj_t *label_current_temperature_value = NULL;
lv_obj_t *label_current_temperature = NULL;
lv_obj_t *label_target_temperature_value = NULL;
lv_obj_t *label_target_temperature = NULL;
lv_obj_t *btnback = NULL;

lv_timer_t *temperatures_screen_delay_timer = NULL;

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
  lv_obj_set_size(btnm_target, MATRIX_BUTTON_WIDTH * i, MATRIX_BUTTON_HEIGHT);
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

void temperatures_screen_delay_timer_cb(lv_timer_t *timer) {
  if (temperatures_screen_delay_timer) {
    lv_timer_del(temperatures_screen_delay_timer);
    temperatures_screen_delay_timer = NULL;
  }
  if (screen_return == ESP3DScreenType::main) {
    mainScreen::main_screen();
  } else if (screen_return == ESP3DScreenType::filament) {
    filamentScreen::filament_screen();
  } else {
    esp3d_log_e("Screen return not supported");
  }
}

void event_button_temperatures_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (temperatures_screen_delay_timer) return;
    temperatures_screen_delay_timer = lv_timer_create(
        temperatures_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    temperatures_screen_delay_timer_cb(NULL);
  }
}

void temperatures_ta_event_cb(lv_event_t *e) {
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
    temperatures_value = lv_textarea_get_text(ta);
    if (std::atoi(temperatures_value.c_str()) > 400) {
      lv_textarea_set_text(ta, "400");
    }
    if (temperatures_value.length() == 0) {
      lv_textarea_set_text(ta, "0");
    }
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    temperatures_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", temperatures_value.c_str());
  }
}

// power_all_heaters_event_cb
void power_all_heaters_event_cb(lv_event_t *e) {
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string temperatures_value = "0";
  int temperatures_int = std::stoi(temperatures_value);
  temperatures_value = std::to_string(temperatures_int);
  lv_textarea_set_text(temperatures_ta, temperatures_value.c_str());
  std::string val =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature);

  if (val == "?") {
    renderingClient.sendGcode("M104 T0 S0");  // power off extruder 0
    renderingClient.sendGcode("M104 T1 S0");  // power off extruder 1
    renderingClient.sendGcode("M140 S0");     // power off bed
  } else {
    for (uint8_t i = 0; i < HEATER_COUNT; i++) {
      if (heater_buttons_visibility_map_id[i] != -1) {
        std::string gcode;
        switch (i) {
          case 0:                  // extruder 0
            gcode = "M104 T0 S0";  // power off current heater
            break;
          case 1:                  // extruder 1
            gcode = "M104 T1 S0";  // power off current heater
            break;
          case 2:               // bed
            gcode = "M140 S0";  // power off current heater
            break;
          default:
            return;
        };
        renderingClient.sendGcode(gcode.c_str());
      }
    }
  }
  esp3d_log("Power off all heaters");
}

// Increase temperature
void temperatures_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string temperatures_value = lv_textarea_get_text(temperatures_ta);
  int temperatures_int = std::stoi(temperatures_value);
  int step = atoi(temperatures_buttons_map[temperatures_buttons_map_id]);
  temperatures_int += step;
  if (temperatures_int > 400) temperatures_int = 400;
  temperatures_value = std::to_string(temperatures_int);
  lv_textarea_set_text(temperatures_ta, temperatures_value.c_str());
}

// Decrease temperature
void temperatures_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string temperatures_value = lv_textarea_get_text(temperatures_ta);
  int temperatures_int = std::stoi(temperatures_value);
  int step = atoi(temperatures_buttons_map[temperatures_buttons_map_id]);
  temperatures_int -= step;
  if (temperatures_int < 0) temperatures_int = 0;
  temperatures_value = std::to_string(temperatures_int);
  lv_textarea_set_text(temperatures_ta, temperatures_value.c_str());
}

void temperatures_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string temperatures_value = lv_textarea_get_text(temperatures_ta);
  esp3d_log("Ok: %s", temperatures_value.c_str());
  std::string gcode;
  uint8_t target = get_heater_buttons_map_id(heater_buttons_map_id);
  switch (target) {
    case 0:                 // extruder 0
      gcode = "M104 T0 S";  // power off current heater
      break;
    case 1:                 // extruder 1
      gcode = "M104 T1 S";  // power off current heater
      break;
    case 2:              // bed
      gcode = "M140 S";  // power off current heater
      break;
    default:
      return;
  };
  gcode += temperatures_value;
  renderingClient.sendGcode(gcode.c_str());
}

void temperatures_btn_power_off_event_cb(lv_event_t *e) {
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Power off current heater %d", heater_buttons_map_id);
  lv_textarea_set_text(temperatures_ta, "0");
  std::string gcode;
  uint8_t target = get_heater_buttons_map_id(heater_buttons_map_id);
  switch (target) {
    case 0:                  // extruder 0
      gcode = "M104 S0 T0";  // power off current heater
      break;
    case 1:                  // extruder 1
      gcode = "M104 S0 T1";  // power off current heater
      break;
    case 2:               // bed
      gcode = "M140 S0";  // power off current heater
      break;
    default:
      return;
  };
  renderingClient.sendGcode(gcode.c_str());
}

void temperatures_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  if (temperatures_buttons_map_id != id) {
    esp3d_log("Button %s clicked", temperatures_buttons_map[id]);
    // update value label
    temperatures_buttons_map_id = id;
  }

  esp3d_log("Button %s clicked", temperatures_buttons_map[id]);
}

void heater_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *temperatures_ta = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    heater_buttons_map_id = id;
    lv_label_set_text(label_current_temperature,
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
    };
    esp3d_log("Button %s clicked", heater_buttons_map[id]);
  }
}

void temperatures_screen(uint8_t target, ESP3DScreenType screenreturn) {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  screen_return = screenreturn;
  if (!intialization_done) {
    esp3d_log("Temperatures screen initialization");
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
    esp3d_log("Temperatures screen initialization done, map size: %d",
              get_map_size());
    get_heater_buttons_map();
  }

  int8_t index = get_heater_buttons_map_button_id(target);
  heater_buttons_map_id = index == -1 ? target : index;

  // Screen creation
  esp3d_log("Temperatures screen creation for target %d, %s", target,
            heater_buttons_label_map[target]);

  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);
  // back button
  btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_temperatures_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm, temperatures_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  size_t i = (sizeof(temperatures_buttons_map) / sizeof(temperatures_buttons_map[0])) - 1;
  lv_obj_set_size(btnm, MATRIX_BUTTON_WIDTH * i, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, temperatures_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, temperatures_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Target selector button matrix
  btnm_target = lv_btnmatrix_create(ui_new_screen);
  // build heater buttons map
  updateBtnMatrix();

  // Power off all heater
  lv_obj_t *btn_power_off_all = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_POWER "...", MATRIX_BUTTON_WIDTH, MATRIX_BUTTON_HEIGHT);
  lv_obj_align_to(btn_power_off_all, btnm_target, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  // Label current heater
  label_current_temperature = lv_label_create(ui_new_screen);
  esp3d_log("Label id: %d", heater_buttons_map_id);
  lv_label_set_text(
      label_current_temperature,
      heater_buttons_map[heater_buttons_map_id]);  // need to change according
                                                   // heater
  apply_style(label_current_temperature, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_temperature, LV_ALIGN_TOP_LEFT,
               CURRENT_BUTTON_PRESSED_OUTLINE, CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_temperature);

  // Label current heater e
  label_current_temperature_value = lv_label_create(ui_new_screen);
  std::string current_temperature_value_init;
  std::string current_temperature_target_value_init;
  switch (target) {
    case 0:
      current_temperature_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature);
      current_temperature_target_value_init = esp3dTftValues.get_string_value(
          ESP3DValuesIndex::ext_0_target_temperature);
      break;
    case 1:
      current_temperature_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_temperature);
      current_temperature_target_value_init = esp3dTftValues.get_string_value(
          ESP3DValuesIndex::ext_1_target_temperature);
      break;
    case 2:
      current_temperature_value_init = esp3dTftValues.get_string_value(
          ESP3DValuesIndex::bed_target_temperature);
      current_temperature_target_value_init = esp3dTftValues.get_string_value(
          ESP3DValuesIndex::bed_target_temperature);
      break;
    default:
      current_temperature_value_init = "0";
      break;
      esp3d_log("Button %s clicked", heater_buttons_label_map[target]);
  }
  if (current_temperature_value_init == "#" ||
      current_temperature_value_init == "?") {
    current_temperature_value_init = "0";
  }
  lv_label_set_text(label_current_temperature_value,
                    current_temperature_value_init.c_str());
  apply_style(label_current_temperature_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_temperature_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_temperature_value, label_current_temperature,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::celsius));
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_temperature_value,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // Button up
  lv_obj_t *btn_up = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn_up, label_current_temperature_value,
                  LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  // Text area
  lv_obj_t *temperatures_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(temperatures_ta, temperatures_ta_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);
  lv_textarea_set_accepted_chars(temperatures_ta, "0123456789");
  lv_textarea_set_max_length(temperatures_ta, 3);
  lv_textarea_set_one_line(temperatures_ta, true);

  std::string temperatures_value_init = std::to_string(
      std::atoi(current_temperature_target_value_init.c_str()) > 0
          ? std::atoi(current_temperature_target_value_init.c_str())
          : 0);
  lv_textarea_set_text(temperatures_ta, temperatures_value_init.c_str());
  lv_obj_set_style_text_align(temperatures_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(temperatures_ta, LV_HOR_RES / 6);

  lv_obj_align_to(temperatures_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btn_up, temperatures_btn_up_event_cb, LV_EVENT_CLICKED,
                      temperatures_ta);
  lv_obj_add_event_cb(btn_power_off_all, power_all_heaters_event_cb,
                      LV_EVENT_CLICKED, temperatures_ta);
  lv_obj_add_event_cb(btnm_target, heater_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, temperatures_ta);

  // Label target heater
  lv_obj_t *label_target = lv_label_create(ui_new_screen);
  lv_label_set_text(label_target,
                    LV_SYMBOL_HEAT_EXTRUDER);  // need to change according
                                               // heater
  apply_style(label_target, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_target, temperatures_ta, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::celsius));
  apply_style(label_unit2, ESP3DStyleType::bg_label);

  lv_obj_align_to(label_unit2, temperatures_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  // set button
  lv_obj_t *btn_set =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_OK);
  lv_obj_align_to(btn_set, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_set, temperatures_btn_ok_event_cb, LV_EVENT_CLICKED,
                      temperatures_ta);
  // Power off button to 0
  lv_obj_t *btn_stop =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_POWER);
  lv_obj_align_to(btn_stop, btn_set, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_stop, temperatures_btn_power_off_event_cb,
                      LV_EVENT_CLICKED, temperatures_ta);
  // Keyboard
  lv_obj_t *temperatures_kb = lv_keyboard_create(ui_new_screen);
  lv_keyboard_set_mode(temperatures_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(temperatures_kb, NULL);
  lv_obj_update_layout(label_unit2);
  lv_obj_set_content_width(temperatures_kb,
                           LV_HOR_RES - (lv_obj_get_x(label_unit2) +
                                         CURRENT_BUTTON_PRESSED_OUTLINE));
  lv_obj_align_to(temperatures_kb, temperatures_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_set_style_radius(temperatures_kb, CURRENT_BUTTON_RADIUS_VALUE,
                          LV_PART_MAIN);
  lv_obj_add_flag(temperatures_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(temperatures_ta, temperatures_ta_event_cb, LV_EVENT_ALL,
                      temperatures_kb);
  // Button down
  lv_obj_t *btn_down = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, temperatures_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, temperatures_btn_down_event_cb,
                      LV_EVENT_CLICKED, temperatures_ta);

  // Label target heater e
  label_target_temperature_value = lv_label_create(ui_new_screen);

  lv_label_set_text(label_target_temperature_value,
                    temperatures_value_init.c_str());
  apply_style(label_target_temperature_value,
              ESP3DStyleType::read_only_setting);
  lv_obj_set_width(label_target_temperature_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_target_temperature_value, btn_down,
                  LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  // unit
  label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::celsius));
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_target_temperature_value,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  // Label target heater
  label_target_temperature = lv_label_create(ui_new_screen);
  lv_label_set_text(
      label_target_temperature,
      heater_buttons_map[heater_buttons_map_id]);  // need to change according
                                                   // heater
  apply_style(label_target_temperature, ESP3DStyleType::bg_label);

  lv_obj_align_to(label_target_temperature, label_target_temperature_value,
                  LV_ALIGN_OUT_LEFT_MID, -CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);

  esp3dTftui.set_current_screen(ESP3DScreenType::temperatures);
}

bool extruder_0_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  // the callback is ignored because the bed callback will be used instead to
  // avoid duplicate code
  return false;
}

bool extruder_1_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  // the callback is ignored because the bed callback will be used instead to
  // avoid duplicate code
  return false;
}

void updateUiValues() {
  uint8_t target = get_heater_buttons_map_id(heater_buttons_map_id);
  if (target == 255) {
    target = 0;
    heater_buttons_map_id = 0;
  }
  switch (target) {
    case 0:
      lv_label_set_text(
          label_current_temperature_value,
          esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature));
      lv_label_set_text(label_target_temperature_value,
                        esp3dTftValues.get_string_value(
                            ESP3DValuesIndex::ext_0_target_temperature));
      break;
    case 1:
      lv_label_set_text(
          label_current_temperature_value,
          esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_temperature));
      lv_label_set_text(label_target_temperature_value,
                        esp3dTftValues.get_string_value(
                            ESP3DValuesIndex::ext_1_target_temperature));
      break;
    case 2:
      lv_label_set_text(
          label_current_temperature_value,
          esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_temperature));
      lv_label_set_text(label_target_temperature_value,
                        esp3dTftValues.get_string_value(
                            ESP3DValuesIndex::bed_target_temperature));
      break;
    default:
      break;
  }
  lv_label_set_text(label_current_temperature,
                    heater_buttons_map[heater_buttons_map_id]);

  lv_label_set_text(label_target_temperature,
                    heater_buttons_map[heater_buttons_map_id]);
}

bool bed_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action) {
  // be sure to be on temperatures screen
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::temperatures)
    return false;
  std::string vale0 =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature);
  std::string vale1 =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_temperature);
  std::string valbed =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_temperature);
  esp3d_log("Bed value callback, %s, %s, %s", vale0.c_str(), vale1.c_str(),
            valbed.c_str());
  esp3d_log("Check if update needed, E0:%s, E1:%s, Bed:%s",
            heater_buttons_visibility_map[0] ? "visible" : "hidden",
            heater_buttons_visibility_map[1] ? "visible" : "hidden",
            heater_buttons_visibility_map[2] ? "visible" : "hidden");
  if (((vale0 != "#") != heater_buttons_visibility_map[0]) ||
      ((vale1 != "#") != heater_buttons_visibility_map[1]) ||
      ((valbed != "#") != heater_buttons_visibility_map[2])) {
    esp3d_log("Update needed");
    heaters_values[0] = vale0;
    heaters_values[1] = vale1;
    heaters_values[2] = valbed;
    updateBtnMatrix();
  }
  updateUiValues();
  return true;
}
}  // namespace temperaturesScreen