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

#include "text_editor_component.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"

/**********************
 *  Namespace
 **********************/
namespace textEditor {
// static variables
std::string inputValue;
lv_obj_t *main_container = nullptr;
void *user_data_ptr = NULL;

/**
 * Callback function for the input area.
 *
 * @param e The event object.
 */
void input_area_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (!lv_obj_is_valid(ta)) {
    esp3d_log_e("Invalid object");
    return;
  }
  void (*callbackFn)(const char *str, void *user_data) =
      (void (*)(const char *, void *))lv_event_get_user_data(e);

  if (code == LV_EVENT_FOCUSED) {
  } else if (code == LV_EVENT_DEFOCUSED) {
    inputValue = lv_textarea_get_text(ta);
  } else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    esp3d_log("Ready, Value: %s", lv_textarea_get_text(ta));
    if (code == LV_EVENT_READY) {
      callbackFn(lv_textarea_get_text(ta), user_data_ptr);
    }
    if (lv_obj_is_valid(main_container)) {
      lv_obj_del(main_container);
    }
    main_container = nullptr;
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    inputValue = lv_textarea_get_text(ta);
    esp3d_log("Value changed: %s", inputValue.c_str());
  }
}

/**
 * @brief Creates a text editor component.
 *
 * This function creates a text editor component and adds it to the specified
 * container.
 *
 * @param container The container object where the text editor will be added.
 * @param text The initial text to be displayed in the text editor.
 * @param callbackFn A function pointer to the callback function that will be
 * called when the text is changed.
 * @param max_length The maximum length of the text that can be entered in the
 * text editor.
 * @param accepted_chars A string containing the characters that are allowed to
 * be entered in the text editor.
 * @param is_number A boolean value indicating whether the text editor should
 * only accept numeric input.
 * @param user_data A pointer to user-defined data that will be passed to the
 * callback function.
 *
 * @return The created text editor object.
 */
lv_obj_t *create(lv_obj_t *container, const char *text,
                 void (*callbackFn)(const char *, void *), size_t max_length,
                 const char *accepted_chars, bool is_number, void *user_data) {
  inputValue = text;
  user_data_ptr = user_data;
  main_container = lv_obj_create(container);
  if (!lv_obj_is_valid(main_container)) {
    esp3d_log_e("Invalid object");
    return nullptr;
  }
  lv_obj_move_foreground(main_container);
  lv_obj_set_size(main_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_t *input_area = lv_textarea_create(main_container);
  lv_obj_update_layout(main_container);
  lv_obj_set_width(input_area, lv_obj_get_content_width(main_container));
  lv_obj_set_x(input_area, 0);
  lv_textarea_set_one_line(input_area, true);
  lv_obj_add_event_cb(input_area, input_area_cb, LV_EVENT_ALL,
                      (void *)callbackFn);
  lv_textarea_set_max_length(input_area, max_length ? max_length : 200);
  esp3d_log("max length: %d", max_length);
  if (accepted_chars) {
    esp3d_log("accepted_chars: %s", accepted_chars);
    lv_textarea_set_accepted_chars(input_area, accepted_chars);
  }

  lv_obj_set_scrollbar_mode(input_area, LV_SCROLLBAR_MODE_AUTO);
  esp3d_log("value: %s", text);
  lv_textarea_set_text(input_area, text);
  lv_obj_t *kb = lv_keyboard_create(main_container);
  if (is_number) {
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
  }
  lv_keyboard_set_textarea(kb, input_area);
  return main_container;
}

}  // namespace textEditor
