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

#include "status_bar_component.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/status_screen.h"

/**********************
 *  Namespace
 **********************/
namespace statusBar {
/**********************
 *  STATIC VARIABLES
 **********************/
lv_obj_t *status_bar_label = nullptr;

/**
 * @brief Callback function for handling ESP3D values.
 *
 * This function is used as a callback for handling ESP3D values. It takes three
 * parameters:
 * - `index`: An enum value representing the index of the ESP3D value.
 * - `value`: A pointer to a character array representing the value of the ESP3D
 * value.
 * - `action`: An enum value representing the action to be performed on the
 * ESP3D value.
 *
 * @param index The index of the ESP3D value.
 * @param value The value of the ESP3D value.
 * @param action The action to be performed on the ESP3D value.
 * @return `true` if the callback was successful, `false` otherwise.
 */
bool callback(ESP3DValuesIndex index, const char *value,
              ESP3DValuesCbAction action) {
  std::string svalue_one_line = esp3d_string::str_replace(value, "\n", "");
  svalue_one_line =
      esp3d_string::str_replace(svalue_one_line.c_str(), "\r", "");
  if (action == ESP3DValuesCbAction::Add ||
      action == ESP3DValuesCbAction::Update) {
    if (status_bar_label != nullptr && lv_obj_is_valid(status_bar_label) &&
        esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      lv_label_set_text(status_bar_label, svalue_one_line.c_str());
    }
  }
  return statusScreen::callback(index, svalue_one_line.c_str(), action);
}

/**
 * @brief Event handler for the status list.
 *
 * This function is responsible for handling events related to the status list
 * component. It is called whenever an event occurs on the status list.
 *
 * @param e Pointer to the event object.
 */
static void event_handler_status_list(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    esp3d_log("Clicked");
    statusScreen::create();
  }
}

/**
 * @brief Creates a status bar component and adds it to the given screen.
 *
 * @param screen The screen to which the status bar component will be added.
 * @return The created status bar component object.
 */
lv_obj_t *create(lv_obj_t *screen) {
  // Create style for status bar
  const ESP3DValuesDescription *status_bar_desc =
      esp3dTftValues.get_description(ESP3DValuesIndex::status_bar_label);

  lv_obj_t *status_bar_container = lv_obj_create(screen);
  if (!lv_obj_is_valid(status_bar_container)) {
    esp3d_log_e("Invalid object");
    return nullptr;
  }
  status_bar_label = lv_label_create(status_bar_container);
  ESP3DStyle::apply(status_bar_label, ESP3DStyleType::status_bar);
  if (status_bar_desc == nullptr) {
    esp3d_log_e("status_bar: description is null cancel");

  } else {
    std::string svalue_one_line =
        esp3d_string::str_replace(status_bar_desc->value.c_str(), "\n", "");
    svalue_one_line =
        esp3d_string::str_replace(svalue_one_line.c_str(), "\r", "");
    lv_label_set_text(status_bar_label, svalue_one_line.c_str());
  }

  // Apply style to status bar

  lv_obj_remove_style_all(status_bar_container);
  lv_obj_set_width(status_bar_container, LV_HOR_RES);
  lv_obj_set_height(status_bar_container, LV_SIZE_CONTENT);
  lv_obj_add_event_cb(status_bar_container, event_handler_status_list,
                      LV_EVENT_CLICKED, NULL);
  return (status_bar_container);
}
}  // namespace statusBar