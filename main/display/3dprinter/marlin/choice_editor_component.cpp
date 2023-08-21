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

#include <string>

#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "symbol_button_component.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace choiceEditor {

std::string choiceValue;
lv_obj_t *main_container = nullptr;
lv_timer_t *choice_editor_delay_timer = NULL;
uint32_t active_index = 0;

void choice_editor_delay_timer_cb(lv_timer_t *timer) {
  void (*callbackFn)(const char *str) =
      (void (*)(const char *))timer->user_data;

  if (choice_editor_delay_timer) {
    lv_timer_del(choice_editor_delay_timer);
    choice_editor_delay_timer = NULL;
  }
  if (main_container) {
    lv_obj_del(main_container);
    main_container = nullptr;
  }
  if (callbackFn) {
    esp3d_log("Ok");
    callbackFn(choiceValue.c_str());
  } else {
    esp3d_log("Cancel");
  }
}

void event_button_choice_editor_handler(lv_event_t *e) {
  void *cbFn = lv_event_get_user_data(e);
  esp3d_log("Button Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (choice_editor_delay_timer) return;
    choice_editor_delay_timer = lv_timer_create(choice_editor_delay_timer_cb,
                                                BUTTON_ANIMATION_DELAY, cbFn);
  } else {
    lv_timer_t timer;
    timer.user_data = cbFn;
    choice_editor_delay_timer_cb(&timer);
  }
}

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

lv_obj_t *create_choice_editor(lv_obj_t *container, const char *text,
                               const char *title,
                               std::list<std::string> &choices,
                               void (*callbackFn)(const char *)) {
  choiceValue = text;

  main_container = lv_obj_create(container);
  lv_obj_move_foreground(main_container);
  lv_obj_set_size(main_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_t *editor_title = lv_label_create(main_container);
  lv_label_set_text(editor_title, title);
  lv_obj_align(editor_title, LV_ALIGN_TOP_MID, 0,
               CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(editor_title);
  size_t y_top = lv_obj_get_y(editor_title) + lv_obj_get_height(editor_title);
  lv_obj_t *btnback = backButton::create_back_button(main_container);
  lv_obj_add_event_cb(btnback, event_button_choice_editor_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btnback);
  size_t y_bottom = lv_obj_get_y(btnback);
  lv_obj_t *btnOk = symbolButton::create_symbol_button(
      main_container, LV_SYMBOL_OK, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT);
  lv_obj_align_to(btnOk, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_obj_add_event_cb(btnOk, event_button_choice_editor_handler,
                      LV_EVENT_CLICKED, (void *)callbackFn);

  lv_obj_t *choice_container = lv_obj_create(main_container);
  size_t heigth = y_bottom - y_top - 2 * CURRENT_BUTTON_PRESSED_OUTLINE;

  apply_style(choice_container, ESP3DStyleType::list_container);

  lv_obj_align_to(choice_container, editor_title, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
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
    lv_checkbox_set_text(obj, choice.c_str());
    lv_obj_update_layout(obj);
    if (!found)
      active_pos += lv_obj_get_height(obj) + CURRENT_BUTTON_PRESSED_OUTLINE;
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    apply_style(obj, ESP3DStyleType::radio_button);
    index++;
  }

  lv_obj_set_size(choice_container, LV_SIZE_CONTENT, heigth);
  lv_obj_update_layout(choice_container);
  size_t width = lv_obj_get_width(choice_container);
  lv_obj_set_x(choice_container, LV_HOR_RES / 2 - width / 2);
  if (heigth < active_pos + CURRENT_BUTTON_PRESSED_OUTLINE)
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
