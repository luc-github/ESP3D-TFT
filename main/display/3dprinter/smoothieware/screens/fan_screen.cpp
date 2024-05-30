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

#include "screens/fan_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/main_screen.h"

/**********************
 *  Namespace
 **********************/
namespace fanScreen {
#define FAN_COUNT 2
#define FAN_LABEL_SIZE 2

// static variables
std::string fan_value = "0";
const char *fan_steps_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t fan_steps_buttons_map_id = 0;
lv_obj_t *label_current_fan_value = NULL;
lv_obj_t *btnm_target = NULL;
lv_obj_t *btnback = NULL;
lv_obj_t *label_current_fan = NULL;
bool intialization_done = false;
uint8_t nb_fans = FAN_COUNT;
char *fan_buttons_map[FAN_COUNT + 1];
uint8_t fan_buttons_map_id = 0;
const char *fan_buttons_label_map[FAN_COUNT] = {LV_SYMBOL_FAN "1",
                                                LV_SYMBOL_FAN "2"};
lv_timer_t *fan_screen_delay_timer = NULL;

/**
 * @brief Get the size of the fan_buttons_map array.
 *
 * This function iterates through the fan_buttons_map array and returns the size
 * of the array. The size is determined by finding the first empty string in the
 * array.
 *
 * @return The size of the fan_buttons_map array.
 */
uint8_t get_map_size() {
  for (uint8_t i = 0; i < FAN_COUNT + 1; i++) {
    if (strlen(fan_buttons_map[i]) == 0) {
      return i;
    }
  }
  esp3d_log("fan_buttons_map size is undefined");
  return 0;
}

/**
 * @brief Retrieves the fan buttons map.
 *
 * This function retrieves the fan buttons map, which is an array of strings
 * representing the labels of the buttons on the fan screen. It checks if the
 * map size matches the number of fans, and if not, it updates the matrix
 * buttons accordingly. Finally, it returns the fan buttons map.
 *
 * @return The fan buttons map as an array of const char pointers.
 */
const char **get_fan_buttons_map() {
  esp3d_log("get_fan_buttons_map");
  esp3d_log("fan_buttons_map size is : %d", get_map_size());

  //  if yes update
  if (get_map_size() != nb_fans) {
    esp3d_log("Update matrix buttons");
    // update matrix buttons
    if (nb_fans == 1) {
      strcpy(fan_buttons_map[1], "");
    } else {
      strcpy(fan_buttons_map[1], fan_buttons_label_map[1]);
    }
  }
  esp3d_log("fan_buttons_map size is : %d", get_map_size());
  // then return
  return (const char **)fan_buttons_map;
}

/**
 * @brief Updates the button matrix on the screen.
 *
 * This function updates the button matrix by setting the button map, applying
 * the style, updating the layout, setting the size, and aligning it to the
 * specified position.
 *
 * @return true if the button matrix was successfully updated, false otherwise.
 */
bool updateBtnMatrix() {
  // check if different from current
  //  if yes update
  // then apply style
  lv_btnmatrix_set_map(btnm_target, get_fan_buttons_map());
  ESP3DStyle::apply(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_target);
  size_t i = get_map_size();
  lv_obj_set_size(btnm_target, ESP3D_MATRIX_BUTTON_WIDTH * i,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  esp3d_log("child count: %d", i);
  // lv_obj_add_state(obj, LV_STATE_DISABLED);
  if (fan_buttons_map_id > i) fan_buttons_map_id = 0;
  lv_btnmatrix_set_btn_ctrl(btnm_target, fan_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  return true;
}

/**
 * Callback function for the fan screen delay timer.
 * This function is called when the delay timer expires.
 * It deletes the timer and creates the main screen.
 *
 * @param timer pointer to the timer object
 */
void fan_screen_delay_timer_cb(lv_timer_t *timer) {
  if (fan_screen_delay_timer && lv_timer_is_valid(fan_screen_delay_timer)) {
    lv_timer_del(fan_screen_delay_timer);
  }
  fan_screen_delay_timer = NULL;
  mainScreen::create();
}

/**
 * @brief Event handler for the "back" button in the fan screen.
 *
 * This function is called when the "back" button is clicked in the fan screen.
 * It logs a message indicating that the button has been clicked and performs
 * some actions based on the value of `ESP3D_BUTTON_ANIMATION_DELAY`.
 *
 * If `ESP3D_BUTTON_ANIMATION_DELAY` is non-zero, it creates a timer with the
 * specified delay and sets `fan_screen_delay_timer` to the created timer.
 * If `ESP3D_BUTTON_ANIMATION_DELAY` is zero, it directly calls the
 * `fan_screen_delay_timer_cb` function.
 *
 * @param e Pointer to the event object.
 */
void event_button_fan_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (fan_screen_delay_timer) return;
    fan_screen_delay_timer = lv_timer_create(
        fan_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    fan_screen_delay_timer_cb(NULL);
  }
}

/**
 * Sends a G-code command to control the fan.
 * The fan value is determined by the `fan_buttons_map_id` and `fan_value`
 * variables. If `fan_buttons_map_id` is 0, the fan is turned off. Otherwise,
 * the fan is turned on. The fan speed is set based on the percentage value in
 * `fan_value`, which is converted to a range of 0-255. The G-code command is
 * sent using the `renderingClient.sendGcode()` function.
 */
void send_gcode_fan() {
  std::string fan_value_str = "M106 P";
  if (fan_buttons_map_id == 0) {
    fan_value_str += "0 S";
  } else {
    fan_value_str += "1 S";
  }
  // convert % to 0-255
  double vfan = (std::stod(fan_value.c_str()) * 255) / 100;
  fan_value_str += esp3d_string::set_precision(std::to_string(vfan), 0);
  renderingClient.sendGcode(fan_value_str.c_str());
}

/**
 * Callback function for the fan text area events.
 *
 * @param e The event object containing information about the event.
 */
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

/**
 * Callback function for the fan up button event.
 * Increases the fan speed by a specified step value.
 *
 * @param e The event object.
 */
void fan_btn_up_event_cb(lv_event_t *e) {
  esp3d_log("Up");
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan);
  int step = atoi(fan_steps_buttons_map[fan_steps_buttons_map_id]);
  esp3d_log("Step: %d, Speed: %d", step, fan_int);
  fan_int += step;
  if (fan_int > 100) fan_int = 100;
  esp3d_log("new Speed: %d", fan_int);
  fan = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan.c_str());
  send_gcode_fan();
}

/**
 * Callback function for the fan button down event.
 * Decreases the fan speed by a specified step value.
 *
 * @param e The event object.
 */
void fan_btn_down_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  std::string fan = lv_textarea_get_text(fan_ta);
  int fan_int = std::stoi(fan);
  int step = atoi(fan_steps_buttons_map[fan_steps_buttons_map_id]);
  fan_int -= step;
  if (fan_int < 0) fan_int = 0;
  fan = std::to_string(fan_int);
  lv_textarea_set_text(fan_ta, fan.c_str());
  send_gcode_fan();
}

/**
 * Callback function for the "OK" button event in the fan screen.
 * This function is called when the "OK" button is clicked.
 * It logs a message and sends a G-code command for controlling the fan.
 *
 * @param e The event object associated with the button click event.
 */
void fan_btn_ok_event_cb(lv_event_t *e) {
  esp3d_log("Ok clicked");
  send_gcode_fan();
}

/**
 * Callback function for the fan reset button event.
 * Resets the fan text area to "0" and sends the corresponding G-code command.
 *
 * @param e The event object.
 */
void fan_btn_reset_event_cb(lv_event_t *e) {
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Reset");
  lv_textarea_set_text(fan_ta, "0");
  send_gcode_fan();
}

/**
 * Callback function for the fan steps matrix buttons event.
 * This function is called when a button in the fan steps matrix is clicked.
 * It retrieves the selected button ID and updates the fan_steps_buttons_map_id
 * variable. It also logs the clicked button's label using esp3d_log function.
 *
 * @param e pointer to the event object
 */
void fan_steps_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  fan_steps_buttons_map_id = id;
  esp3d_log("Button %s clicked", fan_steps_buttons_map[id]);
}

/**
 * Callback function for the fan matrix buttons event.
 *
 * This function is called when a button in the fan matrix is clicked.
 * It updates the selected fan button, updates the label text, and sets the text
 * in the fan textarea.
 *
 * @param e The event object containing information about the event.
 */
void fan_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  // lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *fan_ta = (lv_obj_t *)lv_event_get_user_data(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  fan_buttons_map_id = id;
  lv_label_set_text(label_current_fan, fan_buttons_map[fan_buttons_map_id]);
  std::string temp_value;
  if (fan_buttons_map_id == 0) {
    temp_value = esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan);

  } else {
    temp_value = esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_fan);
  }
  lv_label_set_text(label_current_fan_value, temp_value.c_str());
  lv_textarea_set_text(fan_ta, temp_value.c_str());

  esp3d_log("Button %s clicked", fan_buttons_map[id]);
}

/**
 * @brief Callback function for handling ESP3D values.
 *
 * This function is called when an ESP3D value is updated or changed. It checks
 * the current screen type and performs specific actions based on the value
 * index and action.
 *
 * @param index The index of the ESP3D value.
 * @param value The value of the ESP3D value.
 * @param action The action performed on the ESP3D value.
 * @return True if the callback function handled the value update, false
 * otherwise.
 */
bool callback(ESP3DValuesIndex index, const char *value,
              ESP3DValuesCbAction action) {
  esp3d_log("callback %s", value);
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::fan) {
    esp3d_log("Not current screen");
    return false;
  }
  if (index == ESP3DValuesIndex::ext_1_temperature ||
      index == ESP3DValuesIndex::ext_1_target_temperature) {
    esp3d_log("Check if extruder 1 data %s:", value);
    uint nb_fans_tmp = 2;
    if (strcmp(value, "#") == 0) {
      esp3d_log("No extruder 1, only one fan");
      nb_fans_tmp = 1;
    }
    if (nb_fans_tmp != nb_fans) {
      esp3d_log("Update nb of fans");
      nb_fans = nb_fans_tmp;
    } else {
      // no update needed
      esp3d_log("No update needed");
      return false;
    }
    // update needed for fan buttons
    esp3d_log("Update fan buttons matrix");
    updateBtnMatrix();
    return true;
  }

  esp3d_log("index %d (E0:%d, E1:%d), fan_steps_buttons_map_id %d",
            (uint16_t)index, (uint16_t)ESP3DValuesIndex::ext_0_fan,
            (uint16_t)ESP3DValuesIndex::ext_1_fan, (uint8_t)fan_buttons_map_id);
  if (index == ESP3DValuesIndex::ext_0_fan && fan_buttons_map_id == 0) {
    esp3d_log("Update fan value %s", value);
    lv_label_set_text(label_current_fan_value, value);
    return true;
  }
  if (index == ESP3DValuesIndex::ext_1_fan && fan_buttons_map_id == 1) {
    esp3d_log("Update fan value %s", value);
    lv_label_set_text(label_current_fan_value, value);
    return true;
  }
  esp3d_log("No update needed");
  return false;
}

/**
 * @brief Creates the fan screen.
 *
 * This function is responsible for creating the fan screen in the user
 * interface. It initializes the necessary components and sets up the screen
 * layout.
 *
 * @note This function assumes that the necessary variables and objects have
 * been declared and initialized.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Fan screen creation");
  if (!intialization_done) {
    esp3d_log("fan screen initialization");
    // by default all fans are detected
    //  update will occur only if different
    for (uint8_t i = 0; i < FAN_COUNT; i++) {
      // this one must be called only once or need to free memory
      fan_buttons_map[i] = (char *)calloc(FAN_LABEL_SIZE + 1, sizeof(char));
      strcpy(fan_buttons_map[i], fan_buttons_label_map[i]);
    }
    // the real max final is 0
    fan_buttons_map[FAN_COUNT] = (char *)calloc(1, sizeof(char));
    // last map entry must be empty
    strcpy(fan_buttons_map[FAN_COUNT], "");
    get_fan_buttons_map();
    intialization_done = true;
  }

  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create fan screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_fan_back_handler, LV_EVENT_CLICKED,
                      NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm)) {
    esp3d_log_e("Failed to create button matrix");
    return;
  }
  lv_btnmatrix_set_map(btnm, fan_steps_buttons_map);
  ESP3DStyle::apply(btnm, ESP3DStyleType::buttons_matrix);
  size_t i =
      (sizeof(fan_steps_buttons_map) / sizeof(fan_steps_buttons_map[0])) - 1;
  lv_obj_set_size(btnm, ESP3D_MATRIX_BUTTON_WIDTH * i,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, fan_steps_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_add_event_cb(btnm, fan_steps_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Current Fan label
  label_current_fan = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_fan)) {
    esp3d_log_e("Failed to create current fan label");
    return;
  }
  lv_label_set_text(label_current_fan, fan_buttons_map[fan_buttons_map_id]);
  ESP3DStyle::apply(label_current_fan, ESP3DStyleType::bg_label);
  lv_obj_align(label_current_fan, LV_ALIGN_TOP_LEFT,
               ESP3D_BUTTON_PRESSED_OUTLINE, ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(label_current_fan);

  // Current Fan value
  std::string current_fan_value_init =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan);

  label_current_fan_value = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_fan_value)) {
    esp3d_log_e("Failed to create current fan value label");
    return;
  }
  lv_label_set_text(label_current_fan_value, current_fan_value_init.c_str());
  ESP3DStyle::apply(label_current_fan_value, ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_fan_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_fan_value, label_current_fan,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit1)) {
    esp3d_log_e("Failed to create unit label");
    return;
  }
  lv_label_set_text(label_unit1, "%");
  ESP3DStyle::apply(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_fan_value, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Button up
  lv_obj_t *btn_up =
      symbolButton::create(ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  if (!lv_obj_is_valid(btn_up)) {
    esp3d_log_e("Failed to create up button");
    return;
  }

  lv_obj_align_to(btn_up, label_current_fan_value, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  // fan input
  lv_obj_t *fan_ta = lv_textarea_create(ui_new_screen);
  if (!lv_obj_is_valid(fan_ta)) {
    esp3d_log_e("Failed to create fan text area");
    return;
  }
  lv_obj_add_event_cb(fan_ta, fan_ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_textarea_set_accepted_chars(fan_ta, "0123456789");
  lv_textarea_set_max_length(fan_ta, 3);
  lv_textarea_set_one_line(fan_ta, true);
  lv_textarea_set_text(fan_ta, current_fan_value_init.c_str());
  lv_obj_set_style_text_align(fan_ta, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(fan_ta, LV_HOR_RES / 6);
  lv_obj_align_to(fan_ta, btn_up, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);

  // label
  lv_obj_t *label_ta = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_ta)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_ta, LV_SYMBOL_FAN);
  ESP3DStyle::apply(label_ta, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_ta, fan_ta, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit2)) {
    esp3d_log_e("Failed to create unit label");
    return;
  }
  lv_label_set_text(label_unit2, "%");
  ESP3DStyle::apply(label_unit2, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit2, fan_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // button down
  lv_obj_t *btn_down =
      symbolButton::create(ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);
  if (!lv_obj_is_valid(btn_down)) {
    esp3d_log_e("Failed to create down button");
    return;
  }
  lv_obj_align_to(btn_down, fan_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, fan_btn_down_event_cb, LV_EVENT_CLICKED,
                      fan_ta);

  lv_obj_add_event_cb(btn_up, fan_btn_up_event_cb, LV_EVENT_CLICKED, fan_ta);

  // Button Ok
  lv_obj_t *btn_ok = symbolButton::create(ui_new_screen, LV_SYMBOL_OK);
  if (!lv_obj_is_valid(btn_ok)) {
    esp3d_log_e("Failed to create OK button");
    return;
  }
  lv_obj_align_to(btn_ok, label_unit2, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_ok, fan_btn_ok_event_cb, LV_EVENT_CLICKED, fan_ta);

  // Button Reset
  lv_obj_t *btn_reset = symbolButton::create(ui_new_screen, LV_SYMBOL_POWER);
  if (!lv_obj_is_valid(btn_reset)) {
    esp3d_log_e("Failed to create reset button");
    return;
  }
  lv_obj_align_to(btn_reset, btn_ok, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btn_reset, fan_btn_reset_event_cb, LV_EVENT_CLICKED,
                      fan_ta);

  // keypad
  lv_obj_t *fan_kb = lv_keyboard_create(ui_new_screen);
  if (!lv_obj_is_valid(fan_kb)) {
    esp3d_log_e("Failed to create keyboard");
    return;
  }
  lv_keyboard_set_mode(fan_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(fan_kb, NULL);
  lv_obj_align_to(fan_kb, fan_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(fan_kb);
  lv_obj_set_content_width(fan_kb, LV_HOR_RES - lv_obj_get_x(fan_kb) -
                                       2 * ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(fan_kb, ESP3D_BUTTON_RADIUS, LV_PART_MAIN);
  lv_obj_add_flag(fan_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(fan_ta, fan_ta_event_cb, LV_EVENT_ALL, fan_kb);

  // Target selector button matrix
  btnm_target = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm_target)) {
    esp3d_log_e("Failed to create target button matrix");
    return;
  }
  // build fan buttons map
  updateBtnMatrix();
  lv_obj_add_event_cb(btnm_target, fan_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, fan_ta);

  esp3dTftui.set_current_screen(ESP3DScreenType::fan);
}
}  // namespace fanScreen