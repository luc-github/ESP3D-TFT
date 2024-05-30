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

#include "screens/filament_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/menu_screen.h"
#include "screens/temperatures_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 * Namespace
 **********************/
namespace filamentScreen {
#define FILAMENT_COUNT 2
#define FILAMENT_LABEL_SIZE 2
// Static variables
std::string filament_value = "0";
const char *filament_distance_steps_buttons_map[] = {"1", "5", "10", "50", ""};
uint8_t filament_distance_steps_buttons_map_id = 0;
lv_obj_t *label_current_temperature_filament_value = NULL;
lv_obj_t *btnm_target = NULL;
lv_obj_t *btnback = NULL;
lv_obj_t *label_current_temperature_filament = NULL;
bool intialization_done = false;
uint8_t nb_filaments = FILAMENT_COUNT;
char *filament_buttons_map[FILAMENT_COUNT + 1];
uint8_t filament_buttons_map_id = 0;
const char *filament_buttons_label_map[FILAMENT_COUNT] = {
    LV_SYMBOL_EXTRUDER "1", LV_SYMBOL_EXTRUDER "2"};
lv_timer_t *filament_screen_delay_timer = NULL;

/**
 * @brief Get the size of the filament_buttons_map array.
 *
 * This function iterates through the filament_buttons_map array and returns the
 * size of the array. The size is determined by finding the first empty string
 * in the array.
 *
 * @return The size of the filament_buttons_map array.
 */
uint8_t get_map_size() {
  for (uint8_t i = 0; i < FILAMENT_COUNT + 1; i++) {
    if (strlen(filament_buttons_map[i]) == 0) {
      return i;
    }
  }
  esp3d_log("filament_buttons_map size is undefined");
  return 0;
}

/**
 * @brief Retrieves the filament buttons map.
 *
 * This function retrieves the filament buttons map, which is an array of
 * strings representing the labels of the buttons used for filament selection on
 * a 3D printer display. The size of the map is checked against the number of
 * filaments available, and if they don't match, the map is updated accordingly.
 * If there is only one filament, the second element of the map is set to an
 * empty string. Otherwise, the second element is set to the label of the first
 * filament.
 *
 * @return A pointer to the filament buttons map.
 */
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

/**
 * @brief Updates the button matrix on the filament screen.
 *
 * This function updates the button matrix on the filament screen with the
 * latest button map, applies the appropriate style, and adjusts the size and
 * alignment of the button matrix.
 *
 * @return true if the button matrix was successfully updated, false otherwise.
 */
bool updateBtnMatrix() {
  // check if different from current
  //  if yes update
  // then apply style
  lv_btnmatrix_set_map(btnm_target, get_filament_buttons_map());
  ESP3DStyle::apply(btnm_target, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_target);
  size_t i = get_map_size();
  lv_obj_set_size(btnm_target, ESP3D_MATRIX_BUTTON_WIDTH * i,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  esp3d_log("child count: %d", i);
  // lv_obj_add_state(obj, LV_STATE_DISABLED);
  if (filament_buttons_map_id > i) filament_buttons_map_id = 0;
  lv_btnmatrix_set_btn_ctrl(btnm_target, filament_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  lv_obj_align_to(btnm_target, btnback, LV_ALIGN_OUT_LEFT_BOTTOM,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  return true;
}

/**
 * @brief Callback function for the filament screen delay timer.
 *
 * This function is called when the filament screen delay timer expires. It
 * checks if the timer is valid and deletes it if necessary. Then, it sets the
 * filament_screen_delay_timer to NULL and creates the menu screen.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void filament_screen_delay_timer_cb(lv_timer_t *timer) {
  if (filament_screen_delay_timer &&
      lv_timer_is_valid(filament_screen_delay_timer)) {
    lv_timer_del(filament_screen_delay_timer);
  }
  filament_screen_delay_timer = NULL;
  menuScreen::create();
}

/**
 * @brief Callback function for the filament screen edit delay timer.
 * This function is called when the timer expires.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void filament_screen_edit_delay_timer_cb(lv_timer_t *timer) {
  if (filament_screen_delay_timer &&
      lv_timer_is_valid(filament_screen_delay_timer)) {
    lv_timer_del(filament_screen_delay_timer);
  }
  filament_screen_delay_timer = NULL;
  temperaturesScreen::create(filament_buttons_map_id,
                             ESP3DScreenType::filament);
}

/**
 * @brief Event handler for the "back" button in the filament screen.
 *
 * This function is called when the "back" button is clicked in the filament
 * screen. It logs a message indicating that the button has been clicked and
 * then performs some actions based on the value of
 * ESP3D_BUTTON_ANIMATION_DELAY.
 *
 * If ESP3D_BUTTON_ANIMATION_DELAY is non-zero, it checks if the
 * filament_screen_delay_timer is already created. If it is, the function
 * returns without doing anything. Otherwise, it creates a new timer using
 * lv_timer_create() function with the callback function
 * filament_screen_delay_timer_cb and the delay value
 * ESP3D_BUTTON_ANIMATION_DELAY.
 *
 * If ESP3D_BUTTON_ANIMATION_DELAY is zero, it directly calls the callback
 * function filament_screen_delay_timer_cb with a NULL argument.
 *
 * @param e Pointer to the event object.
 */
void event_button_filament_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (filament_screen_delay_timer) return;
    filament_screen_delay_timer = lv_timer_create(
        filament_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    filament_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief  Event handler for the "edit" button in the filament screen.
 * This function is called when the "edit" button is clicked.
 * It logs a message and creates a timer if necessary.
 * If the animation delay is set, it checks if the delay timer is already
 * running. If not, it creates a new delay timer using the specified callback
 * function and delay duration. If the animation delay is not set, it directly
 * calls the callback function.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_filament_edit_handler(lv_event_t *e) {
  esp3d_log("edit Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (filament_screen_delay_timer) return;
    filament_screen_delay_timer =
        lv_timer_create(filament_screen_edit_delay_timer_cb,
                        ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    filament_screen_edit_delay_timer_cb(NULL);
  }
}

/**
 * @brief Sends G-code commands related to filament control.
 *
 * @param sign The sign to be added to the filament value.
 */
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

/**
 * @brief Callback function for the filament text area event.
 * This function is called when an event occurs on the filament text area.
 * The function handles different event codes and performs corresponding
 * actions.
 *
 * @param e The event object containing information about the event.
 */
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

/**
 * @brief Callback function for the "Up" button event in the filament screen.
 * This function logs the "Up" event and sends a G-code for filament.
 *
 * @param e The event object containing information about the event.
 */
void filament_btn_up_event_cb(lv_event_t *e) {
  esp3d_log("Up");
  send_gcode_filament();
}

/**
 * @brief Callback function for the filament button down event.
 * Sends a G-code command to decrease the filament.
 *
 * @param e Pointer to the event object.
 */
void filament_btn_down_event_cb(lv_event_t *e) { send_gcode_filament("-"); }

/**
 * @brief Callback function for the event triggered by the filament distance
 * steps matrix buttons.
 *
 * @param e The event object.
 */
void filament_distance_steps_matrix_buttons_event_cb(lv_event_t *e) {
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  filament_distance_steps_buttons_map_id = id;
  esp3d_log("Button %s clicked", filament_distance_steps_buttons_map[id]);
  lv_textarea_set_text(ta, filament_distance_steps_buttons_map[id]);
}

/**
 * @brief Callback function for the filament matrix buttons event.
 * This function is called when a button in the filament matrix is clicked.
 * It updates the selected button index, retrieves the current temperature value
 * for the selected filament, and updates the corresponding labels on the
 * screen.
 *
 * @param e The event object containing information about the event.
 */
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

/**
 * @brief Callback function for handling ESP3D values.
 *
 * This function is called when an ESP3D value is received and needs to be
 * processed. It checks the current screen type and the index of the value to
 * determine the action to be taken. If the current screen is not the filament
 * screen, the function returns false. If the index corresponds to the
 * temperature or target temperature of extruder 1, it updates the temperature
 * value on the filament screen and checks the number of filaments. If the value
 * is "#", indicating no extruder 1, it sets the number of filaments to 1. If
 * the number of filaments has changed, it updates the number of filament
 * buttons on the screen. If the index corresponds to the temperature of
 * extruder 0 and the filament distance steps buttons map ID is 0, it updates
 * the filament value on the screen.
 *
 * @param index The index of the ESP3D value.
 * @param value The value of the ESP3D value.
 * @param action The action to be taken for the ESP3D value.
 * @return True if the value was processed and an update was made, false
 * otherwise.
 */
bool callback(ESP3DValuesIndex index, const char *value,
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

/**
 * @brief Creates the filament screen.
 *
 * This function is responsible for creating the filament screen and
 * initializing its components. It sets the current screen to
 * `ESP3DScreenType::none` and then creates a new screen using `lv_obj_create`.
 * The new screen is displayed and the old screen is deleted using `lv_scr_load`
 * and `lv_obj_del` respectively. Various UI elements such as buttons, labels,
 * and text areas are created and positioned on the screen. The filament screen
 * is then set as the current screen using `esp3dTftui.set_current_screen`.
 */
void create() {
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
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create filament screen");
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
  lv_obj_add_event_cb(btnback, event_button_filament_back_handler,
                      LV_EVENT_CLICKED, NULL);

  // Steps in button matrix
  lv_obj_t *btnm = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm)) {
    esp3d_log_e("Failed to create button matrix");
    return;
  }
  lv_btnmatrix_set_map(btnm, filament_distance_steps_buttons_map);
  ESP3DStyle::apply(btnm, ESP3DStyleType::buttons_matrix);
  size_t i = (sizeof(filament_distance_steps_buttons_map) /
              sizeof(filament_distance_steps_buttons_map[0])) -
             1;
  lv_obj_set_size(btnm, ESP3D_MATRIX_BUTTON_WIDTH * i,
                  ESP3D_MATRIX_BUTTON_HEIGHT);
  lv_obj_align(btnm, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_btnmatrix_set_btn_ctrl(btnm, filament_distance_steps_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);

  // Text area label
  lv_obj_t *label_ta = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_ta)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_ta, LV_SYMBOL_FILAMENT);
  ESP3DStyle::apply(label_ta, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_ta);
  int32_t label_width = lv_obj_get_width(label_ta);
  int32_t label_height = lv_obj_get_height(label_ta);
  lv_obj_set_style_transform_pivot_x(label_ta, label_width / 2, 0);
  lv_obj_set_style_transform_pivot_y(label_ta, label_height / 2, 0);
  lv_obj_set_style_transform_angle(label_ta, 900, 0);
  lv_obj_update_layout(label_ta);

  size_t x = lv_obj_get_width(label_ta) + (2 * ESP3D_BUTTON_PRESSED_OUTLINE);
  size_t y = ESP3D_MATRIX_BUTTON_HEIGHT + ESP3D_BUTTON_PRESSED_OUTLINE;

  // Button up
  lv_obj_t *btn_up =
      symbolButton::create(ui_new_screen, LV_SYMBOL_UP "\n" LV_SYMBOL_PLUS);
  if (!lv_obj_is_valid(btn_up)) {
    esp3d_log_e("Failed to create button");
    return;
  }
  lv_obj_set_pos(btn_up, x, y);
  // filament input
  lv_obj_t *filament_ta = lv_textarea_create(ui_new_screen);
  if (!lv_obj_is_valid(filament_ta)) {
    esp3d_log_e("Failed to create text area");
    return;
  }
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
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);

  lv_obj_add_event_cb(btnm, filament_distance_steps_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, filament_ta);

  // Text area label
  lv_obj_align_to(label_ta, filament_ta, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit2 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit2)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_unit2,
                    esp3dTranslationService.translate(ESP3DLabel::millimeters));
  ESP3DStyle::apply(label_unit2, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit2, filament_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // Current filament label
  label_current_temperature_filament = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_temperature_filament)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_current_temperature_filament,
                    filament_buttons_map[filament_buttons_map_id]);
  ESP3DStyle::apply(label_current_temperature_filament,
                    ESP3DStyleType::bg_label);
  lv_obj_align_to(label_current_temperature_filament, label_unit2,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE * 3, 0);
  lv_obj_update_layout(label_current_temperature_filament);

  // Current filament temperature value
  std::string current_temperature_filament_value_init =
      filament_buttons_map_id == 0
          ? esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature)
          : esp3dTftValues.get_string_value(
                ESP3DValuesIndex::ext_1_temperature);

  label_current_temperature_filament_value = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_current_temperature_filament_value)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_current_temperature_filament_value,
                    current_temperature_filament_value_init.c_str());
  ESP3DStyle::apply(label_current_temperature_filament_value,
                    ESP3DStyleType::read_only_value);
  lv_obj_set_width(label_current_temperature_filament_value, LV_HOR_RES / 6);
  lv_obj_align_to(label_current_temperature_filament_value,
                  label_current_temperature_filament, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  // unit
  lv_obj_t *label_unit1 = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_unit1)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_unit1,
                    esp3dTranslationService.translate(ESP3DLabel::celsius));
  ESP3DStyle::apply(label_unit1, ESP3DStyleType::bg_label);
  lv_obj_align_to(label_unit1, label_current_temperature_filament_value,
                  LV_ALIGN_OUT_RIGHT_MID, ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);

  lv_obj_t *btn_edit = symbolButton::create(ui_new_screen, LV_SYMBOL_EDIT);
  if (!lv_obj_is_valid(btn_edit)) {
    esp3d_log_e("Failed to create button");
    return;
  }

  lv_obj_align_to(btn_edit, label_unit1, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_add_event_cb(btn_edit, event_button_filament_edit_handler,
                      LV_EVENT_CLICKED, NULL);

  // button down
  lv_obj_t *btn_down =
      symbolButton::create(ui_new_screen, LV_SYMBOL_MINUS "\n" LV_SYMBOL_DOWN);

  if (!lv_obj_is_valid(btn_down)) {
    esp3d_log_e("Failed to create button");
    return;
  }

  lv_obj_align_to(btn_down, filament_ta, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2);
  lv_obj_add_event_cb(btn_down, filament_btn_down_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  lv_obj_add_event_cb(btn_up, filament_btn_up_event_cb, LV_EVENT_CLICKED,
                      filament_ta);

  // keypad
  lv_obj_t *filament_kb = lv_keyboard_create(ui_new_screen);
  if (!lv_obj_is_valid(filament_kb)) {
    esp3d_log_e("Failed to create keyboard");
    return;
  }
  lv_keyboard_set_mode(filament_kb, LV_KEYBOARD_MODE_NUMBER);
  lv_keyboard_set_textarea(filament_kb, NULL);
  lv_obj_align_to(filament_kb, filament_ta, LV_ALIGN_OUT_RIGHT_MID,
                  ESP3D_BUTTON_PRESSED_OUTLINE / 2, 0);
  lv_obj_update_layout(filament_kb);
  lv_obj_set_content_width(filament_kb, LV_HOR_RES - lv_obj_get_x(filament_kb) -
                                            2 * ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_style_radius(filament_kb, ESP3D_BUTTON_RADIUS, LV_PART_MAIN);
  lv_obj_add_flag(filament_kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(filament_ta, filament_ta_event_cb, LV_EVENT_ALL,
                      filament_kb);

  // Target selector button matrix
  btnm_target = lv_btnmatrix_create(ui_new_screen);
  if (!lv_obj_is_valid(btnm_target)) {
    esp3d_log_e("Failed to create button matrix");
    return;
  }
  // build filament buttons map
  updateBtnMatrix();
  lv_obj_add_event_cb(btnm_target, filament_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, filament_ta);

  esp3dTftui.set_current_screen(ESP3DScreenType::filament);
}
}  // namespace filamentScreen