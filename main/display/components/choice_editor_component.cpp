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

#include "choice_editor_component.h"

#include "back_button_component.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "symbol_button_component.h"

/**********************
 *  Namespace
 **********************/
namespace choiceEditor {

// Static variables
std::string choiceValue;
lv_obj_t *main_container = nullptr;
lv_timer_t *choice_editor_delay_timer = NULL;
uint32_t active_index = 0;
void *user_data_ptr = NULL;

/**
 * @brief Callback function for the choice editor delay timer.
 *
 * This function is called when the delay timer for the choice editor component
 * expires.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void choice_editor_delay_timer_cb(lv_timer_t *timer) {
  void (*callbackFn)(const char *str, void *user_data) =
      (void (*)(const char *, void *))timer->user_data;

  if (choice_editor_delay_timer &&
      lv_timer_is_valid(choice_editor_delay_timer)) {
    lv_timer_del(choice_editor_delay_timer);
  }
  choice_editor_delay_timer = NULL;
  if (main_container && lv_obj_is_valid(main_container)) {
    lv_obj_del(main_container);
  }
  main_container = nullptr;
  if (callbackFn) {
    esp3d_log("Ok");
    callbackFn(choiceValue.c_str(), user_data_ptr);
  } else {
    esp3d_log("Cancel");
  }
}

/**
 * Handles the event for the choice editor button.
 *
 * @param e The event object.
 */
void event_button_choice_editor_handler(lv_event_t *e) {
  void *cbFn = lv_event_get_user_data(e);
  esp3d_log("Button Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (choice_editor_delay_timer) return;
    choice_editor_delay_timer = lv_timer_create(
        choice_editor_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, cbFn);
    if (!lv_timer_is_valid(choice_editor_delay_timer)) {
      esp3d_log_e("Failed to create choice editor delay timer");
      return;
    }

  } else {
    lv_timer_t timer;
    timer.user_data = cbFn;
    choice_editor_delay_timer_cb(&timer);
  }
}

/**
 * Event handler for the choice editor radio button.
 *
 * This function is called when an event occurs on the choice editor radio
 * button. It handles the event and performs the necessary actions based on the
 * event type.
 *
 * @param e Pointer to the event object.
 */
void choice_editor_radio_event_handler(lv_event_t *e) {
  uint32_t *active_id = (uint32_t *)lv_event_get_user_data(e);
  lv_obj_t *cont = lv_event_get_current_target(e);
  lv_obj_t *act_cb = lv_event_get_target(e);
  lv_obj_t *old_cb = lv_obj_get_child(cont, *active_id);

  /*Do nothing if the container was clicked*/
  if (act_cb == cont) return;

  lv_obj_clear_state(old_cb,
                     LV_STATE_CHECKED); /*Uncheck the previous radio button*/
  lv_obj_add_state(act_cb,
                   LV_STATE_CHECKED); /*Uncheck the current radio button*/

  *active_id = lv_obj_get_index(act_cb);

  esp3d_log("Value changed: %s", lv_checkbox_get_text(act_cb));
  choiceValue = lv_checkbox_get_text(act_cb);
}

/**
 * Creates a choice editor component.
 *
 * @param container The parent container object where the component will be
 * created.
 * @param text The text to be displayed on the component.
 * @param title The title of the choice editor component.
 * @param choices A list of choices for the user to select from.
 * @param callbackFn A function pointer to the callback function that will be
 * called when a choice is selected.
 * @param user_data A pointer to user-defined data that will be passed to the
 * callback function.
 * @return The created choice editor component object.
 */
lv_obj_t *create(lv_obj_t *container, const char *text, const char *title,
                 std::list<std::string> &choices,
                 void (*callbackFn)(const char *, void *), void *user_data) {
  choiceValue = text;
  user_data_ptr = user_data;
  main_container = lv_obj_create(container);
  if (!lv_obj_is_valid(main_container)) {
    esp3d_log_e("Failed to create choice editor container");
    return nullptr;
  }
  lv_obj_move_foreground(main_container);
  lv_obj_set_size(main_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_t *editor_title = lv_label_create(main_container);
  if (!lv_obj_is_valid(editor_title)) {
    esp3d_log_e("Failed to create choice editor title");
    return nullptr;
  }
  lv_label_set_text(editor_title, title);
  lv_obj_align(editor_title, LV_ALIGN_TOP_MID, 0, ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(editor_title);
  size_t y_top = lv_obj_get_y(editor_title) + lv_obj_get_height(editor_title);
  lv_obj_t *btnback = backButton::create(main_container);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create choice editor back button");
    return nullptr;
  }
  lv_obj_add_event_cb(btnback, event_button_choice_editor_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btnback);
  size_t y_bottom = lv_obj_get_y(btnback);
  lv_obj_t *btnOk =
      symbolButton::create(main_container, LV_SYMBOL_OK,
                           ESP3D_BACK_BUTTON_WIDTH, ESP3D_BACK_BUTTON_HEIGHT);
  if (!lv_obj_is_valid(btnOk)) {
    esp3d_log_e("Failed to create choice editor OK button");
    return nullptr;
  }
  lv_obj_align_to(btnOk, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btnOk, event_button_choice_editor_handler,
                      LV_EVENT_CLICKED, (void *)callbackFn);

  lv_obj_t *choice_container = lv_obj_create(main_container);
  if (!lv_obj_is_valid(choice_container)) {
    esp3d_log_e("Failed to create choice editor container");
    return nullptr;
  }
  size_t heigth = y_bottom - y_top - 2 * ESP3D_BUTTON_PRESSED_OUTLINE;

  ESP3DStyle::apply(choice_container, ESP3DStyleType::list_container);

  lv_obj_align_to(choice_container, editor_title, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_scrollbar_mode(choice_container, LV_SCROLLBAR_MODE_AUTO);

  uint32_t index = 0;
  size_t active_pos = 0;
  bool found = false;
  for (auto &choice : choices) {
    if (choice == choiceValue) {
      active_index = index;
      found = true;
    }

    lv_obj_t *obj = lv_checkbox_create(choice_container);
    if (!lv_obj_is_valid(obj)) {
      esp3d_log_e("Failed to create choice editor checkbox");
      return nullptr;
    }
    lv_checkbox_set_text(obj, choice.c_str());
    lv_obj_update_layout(obj);
    if (!found)
      active_pos += lv_obj_get_height(obj) + ESP3D_BUTTON_PRESSED_OUTLINE;
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    ESP3DStyle::apply(obj, ESP3DStyleType::radio_button);
    index++;
  }

  lv_obj_set_size(choice_container, ESP3D_CHOICE_LIST_WIDTH, heigth);
  lv_obj_update_layout(choice_container);
  size_t width = lv_obj_get_width(choice_container);
  lv_obj_set_x(choice_container, LV_HOR_RES / 2 - width / 2);
  if (heigth < active_pos + ESP3D_BUTTON_PRESSED_OUTLINE)
    lv_obj_scroll_to_y(choice_container, active_pos, LV_ANIM_ON);
  // check first one by default
  lv_obj_add_state(lv_obj_get_child(choice_container, active_index),
                   LV_STATE_CHECKED);
  lv_obj_add_event_cb(choice_container, choice_editor_radio_event_handler,
                      LV_EVENT_PRESSED, &active_index);
  lv_obj_add_event_cb(choice_container, choice_editor_radio_event_handler,
                      LV_EVENT_CLICKED, &active_index);
  return main_container;
}

}  // namespace choiceEditor
