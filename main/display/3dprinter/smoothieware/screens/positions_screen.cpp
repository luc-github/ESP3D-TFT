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

#include "screens/positions_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/main_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/

namespace positionsScreen {
// Static variables
std::string position_value = "0.00";
const char *positions_buttons_map[] = {"0.1", "1", "10", "50", ""};
uint8_t positions_buttons_map_id = 0;
bool absolute_position = false;
const char *axis_buttons_map[] = {"X", "Y", "Z", ""};
uint8_t axis_buttons_map_id = 0;
lv_obj_t *label_current_position_value = NULL;
lv_obj_t *label_current_position = NULL;
lv_timer_t *positions_screen_delay_timer = NULL;
lv_obj_t *position_ta = NULL;
lv_obj_t *btn_set = NULL;
bool intialization_done = false;

// Static functions
/**
 * @brief Callback function for handling updates to position values.
 *
 * This function is called when there is an update to the position values in the
 * ESP3D system. It checks the current screen and the index of the value being
 * updated, and updates the corresponding label on the screen if necessary.
 *
 * @param index The index of the value being updated.
 * @param value The new value of the position.
 * @param action The action to be performed on the value.
 * @return True if the callback was handled successfully, false otherwise.
 */
bool callback(ESP3DValuesIndex index, const char *value,
              ESP3DValuesCbAction action) {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::positions)
    return false;

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

/**
 * @brief Callback function for the delay timer in the positions screen.
 *
 * This function is called when the delay timer expires. It is responsible for
 * deleting the timer and creating the main screen.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void positions_screen_delay_timer_cb(lv_timer_t *timer) {
  if (positions_screen_delay_timer &&
      lv_timer_is_valid(positions_screen_delay_timer)) {
    lv_timer_del(positions_screen_delay_timer);
  }
  positions_screen_delay_timer = NULL;
  mainScreen::create();
}

/**
 * @brief Handles the event when the back button is clicked on the positions
 * screen.
 *
 * @param e Pointer to the event object.
 */
void event_button_positions_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (positions_screen_delay_timer) return;
    positions_screen_delay_timer = lv_timer_create(
        positions_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    positions_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Callback function for the position text area events.
 *
 * This function is called when an event occurs on the position text area.
 * It handles different events such as focus, defocus, ready, cancel, and value
 * change.
 *
 * @param e Pointer to the event object.
 */
void position_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_PRESSED) {
    esp3d_log("Clicked, Value");
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
  } else if (code == LV_EVENT_DEFOCUSED) {
    esp3d_log("Defocused, Value");
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    position_value = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready/Cancel, Value: %s", lv_textarea_get_text(ta));
    position_value = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", position_value.c_str());
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    //
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    // position_value = lv_textarea_get_text(ta);
    // esp3d_log("Value changed: %s", position_value.c_str());
  }
}

/**
 * @brief Sends a G-code command to the rendering client based on the given
 * position value.
 *
 * @param position_value The position value to be included in the G-code
 * command.
 */
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

/**
 * @brief Callback function for the "up" button event in the positions screen.
 * This function is triggered when the "up" button is pressed.
 * It retrieves the current position value from a textarea object,
 * increments the position value by a step value, and updates the textarea with
 * the new position value. If the position is absolute, it also sends a G-code
 * command with the new position value.
 *
 * @param e The event object associated with the button press event.
 */
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
        esp3d_string::set_precision(std::to_string(position_double), 2);
    lv_textarea_set_text(position_ta, position_value.c_str());
  }
  send_gcode_position(position_value.c_str());
}

/**
 * @brief Callback function for the button down event in the positions screen.
 * Decreases the position value based on the step size and updates the position
 * text area. If the position is in absolute mode, the position value is
 * decreased by the step size. If the position is in relative mode, the position
 * value is negated. Finally, the updated position value is sent as a G-code
 * command.
 *
 * @param e The event object containing information about the button down event.
 */
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
        esp3d_string::set_precision(std::to_string(position_double), 2);
    lv_textarea_set_text(position_ta, position_value.c_str());
  } else {
    position_value = "-" + position_value;
  }
  send_gcode_position(position_value.c_str());
}

/**
 * @brief Callback function for the "Ok" button event in the positions screen.
 * Retrieves the text from a textarea object and sends it as a G-code position
 * command.
 *
 * @param e The event object.
 */
void positions_btn_ok_event_cb(lv_event_t *e) {
  lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string position_value = lv_textarea_get_text(position_ta);
  esp3d_log("Ok: %s", position_value.c_str());
  send_gcode_position(position_value.c_str());
}

/**
 * @brief Callback function for the "Home Axis" button event.
 * This function is called when the "Home Axis" button is pressed.
 * It sends a G28 command to the rendering client to home the specified axis.
 *
 * @param e The event object associated with the button press.
 */
void positions_btn_home_axis_event_cb(lv_event_t *e) {
  // lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Home Axis %c", axis_buttons_map[axis_buttons_map_id][0]);
  std::string gcode = "G28 ";
  gcode += axis_buttons_map[axis_buttons_map_id];
  renderingClient.sendGcode(gcode.c_str());
}

/**
 * @brief Updates the mode of the positions screen.
 *
 * This function is responsible for updating the mode of the positions screen
 * based on the given parameter. If the mode is true, the position text area is
 * enabled and its text is set to the current position value. If the mode is
 * false, the position text area is disabled and its text is set to a predefined
 * value.
 *
 * @param mode The mode to set for the positions screen.
 */
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
/**
 * @brief Updates the mode label with the appropriate text based on the mode.
 *
 * @param mode The mode indicating whether it is in absolute or relative mode.
 * @param label The label object to be updated.
 */
void updateModeLabel(bool mode, lv_obj_t *label) {
  std::string absstr =
      esp3dTranslationService.translate(ESP3DLabel::absolute_short);
  std::string relstr =
      esp3dTranslationService.translate(ESP3DLabel::relative_short);
  std::string text;
  if (mode) {
    text = "#00ff00 " + absstr + "# / " + relstr;
  } else {
    text = absstr + " / #00ff00 " + relstr + "#";
  }

  lv_label_set_text(label, text.c_str());
}

/**
 * @brief Callback function for the mode button event.
 *
 * This function is called when the mode button is pressed. It toggles the value
 * of the boolean pointer passed as user data, updates the mode label, and calls
 * the updateMode function with the new value.
 *
 * @param e The event object.
 */
void btn_mode_event_cb(lv_event_t *e) {
  bool *babsolute = (bool *)lv_event_get_user_data(e);
  *babsolute = !(*babsolute);
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(obj, 0);
  updateModeLabel(*babsolute, label);
  updateMode(*babsolute);
}

/**
 * @brief Callback function for the "Home All Axis" button event.
 * This function is called when the button is clicked.
 * It sends the G-code command "G28" to the rendering client to home all axes.
 *
 * @param e The event object containing information about the event.
 */
void home_all_axis_button_event_cb(lv_event_t *e) {
  // lv_obj_t *position_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Home All Axis");
  renderingClient.sendGcode("G28");
}

/**
 * @brief Callback function for the positions matrix buttons event.
 *
 * This function is called when a button in the positions matrix is clicked.
 * It retrieves the selected button ID and updates the positions_buttons_map_id
 * variable. It also logs the clicked button and checks if the absolute_position
 * flag is false, then calls the updateMode function.
 *
 * @param e Pointer to the event object.
 */
void positions_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  positions_buttons_map_id = id;
  esp3d_log("Button %s clicked", positions_buttons_map[id]);
  if (!absolute_position) updateMode(absolute_position);
}

/**
 * @brief Callback function for handling events from the axis matrix buttons.
 *
 * @param e The event object containing information about the event.
 */
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
  std::string position_value_init = esp3d_string::set_precision(
      current_position_value_init == "?" ? "0.00" : current_position_value_init,
      2);
  lv_textarea_set_text(position_ta, position_value_init.c_str());
}

/**
 * @brief Creates a new object with the specified target ID.
 *
 * @param target_id The ID of the target.
 */
void create(uint8_t target_id) {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  if (target_id != 255) {
    axis_buttons_map_id = target_id;
  }
  if (!intialization_done) {
    absolute_position =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_jog_type) == 0
            ? false
            : true;
    intialization_done = true;
  }
  //  Screen creation
  esp3d_log("Positions screen creation for target %d", axis_buttons_map_id);
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Error creating screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  // back button
  lv_obj_t *btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Error creating back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_positions_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm)) {
    esp3d_log_e("Error creating button matrix");
    return;
  }
  lv_btnmatrix_set_map(btnm, positions_buttons_map);
  ESP3DStyle::apply(btnm, ESP3DStyleType::buttons_matrix);
  size_t i =
      (sizeof(positions_buttons_map) / sizeof(positions_buttons_map[0])) - 1;
  lv_obj_set_size(btnm, ESP3D_MATRIX_BUTTON_WIDTH * i,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, positions_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, positions_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Target selector button matrix
  lv_obj_t *btnm_target = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm_target)) {
    esp3d_log_e("Error creating button matrix");
    return;
  }
  lv_btnmatrix_set_map(btnm_target, axis_buttons_map);
  ESP3DStyle::apply(btnm_target, ESP3DStyleType::buttons_matrix);
  size_t i2 = (sizeof(axis_buttons_map) / sizeof(axis_buttons_map[0])) - 1;
  lv_obj_set_size(btnm_target, ESP3D_MATRIX_BUTTON_WIDTH * i2,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  lv_btnmatrix_set_btn_ctrl(btnm_target, axis_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);

  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  // Home all axis
  lv_obj_t *btn_home_all = symbolButton::create(
      ui_new_screen, LV_SYMBOL_HOME "xyz", ESP3D_MATRIX_BUTTON_WIDTH,
      ESP3D_MATRIX_BUTTON_HEIGHT);
  if (!lv_obj_is_valid(btn_home_all)) {
    esp3d_log_e("Error creating home all button");
    return;
  }
  lv_obj_align_to(btn_home_all, btnm_target, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);

  // Label current axis
  label_current_position = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_position)) {
    esp3d_log_e("Error creating label");
    return;
  }
  lv_label_set_text(label_current_position,
                    axis_buttons_map[axis_buttons_map_id]);  // need to change
                                                             // according axis
  ESP3DStyle::apply(label_current_position, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_position, LV_ALIGN_TOP_LEFT,
               ESP3D_BUTTON_PRESSED_OUTLINE, ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_position);

  // Label current axis e
  label_current_position_value = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_position_value)) {
    esp3d_log_e("Error creating label");
    return;
  }
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

  ESP3DStyle::apply(label_current_position_value,
                    ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_position_value, LV_HOR_RES / 5);
  lv_obj_align_to(label_current_position_value, label_current_position,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit1)) {
    esp3d_log_e("Error creating label");
    return;
  }
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  ESP3DStyle::apply(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_position_value,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  // Button up
  lv_obj_t *btn_up =
      symbolButton::create(ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  if (!lv_obj_is_valid(btn_up)) {
    esp3d_log_e("Error creating button");
    return;
  }
  lv_obj_align_to(btn_up, label_current_position_value, LV_ALIGN_OUT_BOTTOM_MID,
                  0, ESP3D_BUTTON_PRESSED_OUTLINE);
  // Text area
  position_ta = lv_textarea_create(ui_new_screen);
  if (!lv_obj_is_valid(position_ta)) {
    esp3d_log_e("Error creating text area");
    return;
  }
  lv_obj_add_event_cb(position_ta, position_ta_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_obj_add_event_cb(btnm_target, axis_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, position_ta);
  lv_textarea_set_accepted_chars(position_ta, "0123456789.-");
  lv_textarea_set_max_length(position_ta, 7);
  lv_textarea_set_one_line(position_ta, true);
  esp3d_log("value: %s", position_value.c_str());
  std::string position_value_init = esp3d_string::set_precision(
      current_position_value_init == "?" ? "0.00" : current_position_value_init,
      2);
  lv_textarea_set_text(position_ta, position_value_init.c_str());
  lv_obj_set_style_text_align(position_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(position_ta, LV_HOR_RES / 5);

  lv_obj_align_to(position_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btn_up, positions_btn_up_event_cb, LV_EVENT_CLICKED,
                      position_ta);

  // Label target axis
  lv_obj_t *label_target = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_target)) {
    esp3d_log_e("Error creating label");
    return;
  }
  lv_label_set_text(label_target,
                    LV_SYMBOL_JOG);  // need to change according
                                     // axis
  ESP3DStyle::apply(label_target, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_target, position_ta, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit2)) {
    esp3d_log_e("Error creating label");
    return;
  }
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  ESP3DStyle::apply(label_unit2, ESP3DStyleType::bg_label);

  lv_obj_align_to(label_unit2, position_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  // set button
  btn_set = symbolButton::create(ui_new_screen, LV_SYMBOL_OK);
  if (!lv_obj_is_valid(btn_set)) {
    esp3d_log_e("Error creating button");
    return;
  }
  lv_obj_align_to(btn_set, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_set, positions_btn_ok_event_cb, LV_EVENT_CLICKED,
                      position_ta);
  // Home axis
  lv_obj_t *btn_home_axis = symbolButton::create(ui_new_screen, LV_SYMBOL_HOME);
  if (!lv_obj_is_valid(btn_home_axis)) {
    esp3d_log_e("Error creating button");
    return;
  }
  lv_obj_align_to(btn_home_axis, btn_set, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_home_axis, positions_btn_home_axis_event_cb,
                      LV_EVENT_CLICKED, position_ta);
  lv_obj_add_event_cb(btn_home_all, home_all_axis_button_event_cb,
                      LV_EVENT_CLICKED, position_ta);

  // absolute /relative mode button
  lv_obj_t *btn_mode = lv_btn_create(ui_new_screen);
  if (!lv_obj_is_valid(btn_mode)) {
    esp3d_log_e("Error creating button");
    return;
  }
  ESP3DStyle::apply(btn_mode, ESP3DStyleType::button);
  lv_obj_t *label = lv_label_create(btn_mode);
  lv_label_set_recolor(label, true);
  updateModeLabel(absolute_position, label);

  lv_obj_center(label);
  lv_obj_add_event_cb(btn_mode, btn_mode_event_cb, LV_EVENT_CLICKED,
                      &absolute_position);

  lv_obj_align_to(btn_mode, btnm, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);

  // Keyboard
  lv_obj_t *positions_kb = lv_keyboard_create(ui_new_screen);
  if (!lv_obj_is_valid(positions_kb)) {
    esp3d_log_e("Error creating keyboard");
    return;
  }
  lv_keyboard_set_mode(positions_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(positions_kb, NULL);
  lv_obj_update_layout(label_unit2);
  lv_obj_set_content_width(
      positions_kb,
      LV_HOR_RES - (lv_obj_get_x(label_unit2) + ESP3D_BUTTON_PRESSED_OUTLINE));
  lv_obj_align_to(positions_kb, position_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2,
                  -ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_set_style_radius(positions_kb, ESP3D_BUTTON_RADIUS, LV_PART_MAIN);
  lv_obj_add_flag(positions_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(position_ta, position_ta_event_cb, LV_EVENT_ALL,
                      positions_kb);
  // Button down
  lv_obj_t *btn_down =
      symbolButton::create(ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  if (!lv_obj_is_valid(btn_down)) {
    esp3d_log_e("Error creating button");
    return;
  }
  lv_obj_align_to(btn_down, position_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, positions_btn_down_event_cb, LV_EVENT_CLICKED,
                      position_ta);
  updateMode(absolute_position);
  esp3dTftui.set_current_screen(ESP3DScreenType::positions);
}
}  // namespace positionsScreen