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

#include "positions_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "main_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "translations/esp3d_translation_service.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/

namespace positionsScreen {
std::string position_value = "0.00";
const char *positions_buttons_map[] = {"0.1", "1", "10", "50", ""};
uint8_t positions_buttons_map_id = 0;

bool absolute_position = true;

const char *axis_buttons_map[] = {"X", "Y", "Z", ""};

uint8_t axis_buttons_map_id = 0;

lv_obj_t *label_current_position_value = NULL;
lv_obj_t *label_current_position = NULL;
lv_timer_t *positions_screen_delay_timer = NULL;
lv_obj_t *position_ta = NULL;
lv_obj_t *btn_set = NULL;

bool positions_values_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  switch (index) {
    case ESP3DValuesIndex::position_x:
      if (action == ESP3DValuesCbAction::Update && axis_buttons_map_id == 0) {
        lv_label_set_text(label_current_position_value, value);
      }
      break;
    case ESP3DValuesIndex::position_y:
      if (action == ESP3DValuesCbAction::Update && axis_buttons_map_id == 1) {
        lv_label_set_text(label_current_position_value, value);
      }
      break;
    case ESP3DValuesIndex::position_z:
      if (action == ESP3DValuesCbAction::Update && axis_buttons_map_id == 2) {
        lv_label_set_text(label_current_position_value, value);
      }
      break;
    default:
      break;
  }
  return true;
}

void positions_screen_delay_timer_cb(lv_timer_t *timer) {
  if (positions_screen_delay_timer) {
    lv_timer_del(positions_screen_delay_timer);
    positions_screen_delay_timer = NULL;
  }
  mainScreen::main_screen();
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

void position_ta_event_cb(lv_event_t *e) {
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
    position_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    // No idea how to change ta focus to another control
    // every tests I did failed
    // so I refresh all screen ... orz
    position_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", position_value.c_str());
    positions_screen(axis_buttons_map_id);
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    // position_value = lv_textarea_get_text(ta);
    // esp3d_log("Value changed: %s", position_value.c_str());
  }
}
void send_gcode_position(const char *position_value) {
  if (absolute_position) {
    std::string gcode = "G1 ";
    gcode += axis_buttons_map[axis_buttons_map_id];
    gcode += position_value;
    renderingClient.sendGcode(gcode.c_str());
  } else {
    std::string gcode = "G91";
    renderingClient.sendGcode(gcode.c_str());
    gcode = "G1 ";
    gcode += axis_buttons_map[axis_buttons_map_id];
    gcode += position_value;
    renderingClient.sendGcode(gcode.c_str());
    gcode = "G90";
    renderingClient.sendGcode(gcode.c_str());
  }
}

void positions_btn_up_event_cb(lv_event_t *e) {
  lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string position_value = lv_textarea_get_text(position_ta);
  if (absolute_position) {
    double position_double = std::stod(position_value);
    double step = std::stod(positions_buttons_map[positions_buttons_map_id]);
    esp3d_log("Current pos: %s,  %f", position_value.c_str(), position_double);
    position_double += step;
    esp3d_log("Up: %f, new pos: %f", step, position_double);
    position_value =
        esp3d_strings::set_precision(std::to_string(position_double), 2);
    lv_textarea_set_text(position_ta, position_value.c_str());
  }
  send_gcode_position(position_value.c_str());
}

void positions_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string position_value = lv_textarea_get_text(position_ta);
  if (absolute_position) {
    double position_double = std::stod(position_value);
    double step = std::stod(positions_buttons_map[positions_buttons_map_id]);
    esp3d_log("Current pos: %s,  %f", position_value.c_str(), position_double);
    position_double -= step;
    esp3d_log("Down: %f, new pos: %f", step, position_double);
    position_value =
        esp3d_strings::set_precision(std::to_string(position_double), 2);
    lv_textarea_set_text(position_ta, position_value.c_str());
  } else {
    position_value = "-" + position_value;
  }
  send_gcode_position(position_value.c_str());
}

void positions_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string position_value = lv_textarea_get_text(position_ta);
  esp3d_log("Ok: %s", position_value.c_str());
  send_gcode_position(position_value.c_str());
}

void positions_btn_home_axis_event_cb(lv_event_t *e) {
  // lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Home Axis %c", axis_buttons_map[axis_buttons_map_id][0]);
  std::string gcode = "G28 ";
  gcode += axis_buttons_map[axis_buttons_map_id];
  renderingClient.sendGcode(gcode.c_str());
}

void updateMode(bool mode) {
  if (mode) {
    lv_obj_clear_state(position_ta, LV_STATE_DISABLED);
    std::string current_position_value =
        lv_label_get_text(label_current_position_value);
    lv_textarea_set_text(position_ta, current_position_value == "?"
                                          ? "0.00"
                                          : current_position_value.c_str());
    lv_obj_clear_flag(btn_set, LV_OBJ_FLAG_HIDDEN);

  } else {
    lv_obj_add_flag(btn_set, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(position_ta, LV_STATE_DISABLED);
    lv_textarea_set_text(position_ta,
                         positions_buttons_map[positions_buttons_map_id]);
  }
}
void updateModeLabel(bool mode, lv_obj_t *label) {
  std::string absstr = esp3dTranslationService.translate(ESP3DLabel::absolute);
  std::string relstr = esp3dTranslationService.translate(ESP3DLabel::relative);
  std::string text;
  if (mode) {
    text = "#00ff00 " + absstr + "# / " + relstr;
  } else {
    text = absstr + " / #00ff00 " + relstr + "#";
  }

  lv_label_set_text(label, text.c_str());
}

void btn_mode_event_cb(lv_event_t *e) {
  bool *babsolute = (bool *)lv_event_get_user_data(e);
  *babsolute = !(*babsolute);
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(obj, 0);
  updateModeLabel(*babsolute, label);
  updateMode(*babsolute);
}
void home_all_axis_button_event_cb(lv_event_t *e) {
  // lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Home All Axis");
  renderingClient.sendGcode("G28");
}

void positions_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  positions_buttons_map_id = id;
  esp3d_log("Button %s clicked", positions_buttons_map[id]);
  if (!absolute_position) updateMode(absolute_position);
}

void axis_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  axis_buttons_map_id = id;
  lv_label_set_text(label_current_position,
                    axis_buttons_map[axis_buttons_map_id]);
  std::string current_position_value_init;
  switch (axis_buttons_map_id) {
    case 0:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_x);
      break;
    case 1:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_y);
      break;
    case 2:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_z);
      break;
    default:
      current_position_value_init = "0.00";
      break;
      esp3d_log("Button %s clicked", axis_buttons_map[id]);
  }
  lv_label_set_text(label_current_position_value,
                    current_position_value_init.c_str());
  if (!absolute_position) return;
  std::string position_value_init = esp3d_strings::set_precision(
      current_position_value_init == "?" ? "0.00" : current_position_value_init,
      2);
  lv_textarea_set_text(position_ta, position_value_init.c_str());
}

void positions_screen(uint8_t target_id) {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  if (target_id != 255) {
    axis_buttons_map_id = target_id;
  }
  //  Screen creation
  esp3d_log("Positions screen creation for target %d", axis_buttons_map_id);
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);
  // back button
  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_positions_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm, positions_buttons_map);
  apply_style(btnm, ESP3DStyleType::buttons_matrix);
  lv_obj_set_size(btnm, LV_HOR_RES / 2, MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -CURRENT_BUTTON_PRESSED_OUTLINE,
               CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, positions_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, positions_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Target selector button matrix
  lv_obj_t *btnm_target = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm_target, axis_buttons_map);
  apply_style(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_set_height(btnm_target, MATRIX_BUTTON_HEIGHT);
  lv_btnmatrix_set_btn_ctrl(btnm_target, axis_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);

  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  // Home all axis
  lv_obj_t *btn_home_all = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_HOME "xyz", -1, MATRIX_BUTTON_HEIGHT);
  lv_obj_align_to(btn_home_all, btnm_target, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  // Label current axis
  label_current_position = lv_label_create(ui_new_screen);
  lv_label_set_text(label_current_position,
                    axis_buttons_map[axis_buttons_map_id]);  // need to change
                                                             // according axis
  apply_style(label_current_position, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_position, LV_ALIGN_TOP_LEFT,
               CURRENT_BUTTON_PRESSED_OUTLINE, CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_position);

  // Label current axis e
  label_current_position_value = lv_label_create(ui_new_screen);

  std::string current_position_value_init;
  switch (axis_buttons_map_id) {
    case 0:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_x);
      break;
    case 1:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_y);
      break;
    case 2:
      current_position_value_init =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::position_z);
      break;
    default:
      current_position_value_init = "0.00";
      break;
  }

  lv_label_set_text(label_current_position_value,
                    current_position_value_init.c_str());

  apply_style(label_current_position_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_position_value, LV_HOR_RES / 5);
  lv_obj_align_to(label_current_position_value, label_current_position,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  apply_style(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_position_value,
                  LV_ALIGN_OUT_RIGHT_MID, CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  0);
  // Button up
  lv_obj_t *btn_up = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  lv_obj_align_to(btn_up, label_current_position_value, LV_ALIGN_OUT_BOTTOM_MID,
                  0, CURRENT_BUTTON_PRESSED_OUTLINE);
  // Text area
  position_ta = lv_textarea_create(ui_new_screen);
  lv_obj_add_event_cb(position_ta, position_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_obj_add_event_cb(btnm_target, axis_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, position_ta);
  lv_textarea_set_accepted_chars(position_ta, "0123456789.-");
  lv_textarea_set_max_length(position_ta, 7);
  lv_textarea_set_one_line(position_ta, true);
  esp3d_log("value: %s", position_value.c_str());
  std::string position_value_init = esp3d_strings::set_precision(
      current_position_value_init == "?" ? "0.00" : current_position_value_init,
      2);
  lv_textarea_set_text(position_ta, position_value_init.c_str());
  lv_obj_set_style_text_align(position_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(position_ta, LV_HOR_RES / 5);

  lv_obj_align_to(position_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btn_up, positions_btn_up_event_cb, LV_EVENT_CLICKED,
                      position_ta);

  // Label target axis
  lv_obj_t *label_target = lv_label_create(ui_new_screen);
  lv_label_set_text(label_target,
                    LV_SYMBOL_JOG);  // need to change according
                                     // axis
  apply_style(label_target, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_target, position_ta, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  apply_style(label_unit2, ESP3DStyleType::bg_label);

  lv_obj_align_to(label_unit2, position_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2, 0);
  // set button
  btn_set = symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_OK);
  lv_obj_align_to(btn_set, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_set, positions_btn_ok_event_cb, LV_EVENT_CLICKED,
                      position_ta);
  // Home axis
  lv_obj_t *btn_home_axis =
      symbolButton::create_symbol_button(ui_new_screen, LV_SYMBOL_HOME);
  lv_obj_align_to(btn_home_axis, btn_set, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_home_axis, positions_btn_home_axis_event_cb,
                      LV_EVENT_CLICKED, position_ta);
  lv_obj_add_event_cb(btn_home_all, home_all_axis_button_event_cb,
                      LV_EVENT_CLICKED, position_ta);

  // absolute /relative mode button
  lv_obj_t *btn_mode = lv_btn_create(ui_new_screen);
  apply_style(btn_mode, ESP3DStyleType::button);
  lv_obj_t *label = lv_label_create(btn_mode);
  lv_label_set_recolor(label, true);
  updateModeLabel(absolute_position, label);

  lv_obj_center(label);
  lv_obj_add_event_cb(btn_mode, btn_mode_event_cb, LV_EVENT_CLICKED,
                      &absolute_position);

  lv_obj_align_to(btn_mode, btnm, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);

  // Keyboard
  lv_obj_t *positions_kb = lv_keyboard_create(ui_new_screen);
  lv_keyboard_set_mode(positions_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(positions_kb, NULL);
  lv_obj_update_layout(label_unit2);
  lv_obj_set_content_width(positions_kb,
                           LV_HOR_RES - (lv_obj_get_x(label_unit2) +
                                         CURRENT_BUTTON_PRESSED_OUTLINE));
  lv_obj_align_to(positions_kb, position_ta, LV_ALIGN_OUT_RIGHT_MID,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2,
                  -CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_set_style_radius(positions_kb, CURRENT_BUTTON_RADIUS_VALUE,
                          LV_PART_MAIN);
  lv_obj_add_flag(positions_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(position_ta, position_ta_event_cb, LV_EVENT_ALL,
                      positions_kb);
  // Button down
  lv_obj_t *btn_down = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  lv_obj_align_to(btn_down, position_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, positions_btn_down_event_cb, LV_EVENT_CLICKED,
                      position_ta);
  updateMode(absolute_position);
  esp3dTftui.set_current_screen(ESP3DScreenType::positions);
}
}  // namespace positionsScreen