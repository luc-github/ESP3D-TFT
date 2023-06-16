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

#include <list>
#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_tft_ui.h"
#include "esp3d_values.h"
#include "lvgl.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
// Create style for buttons
lv_style_t style_button_back;
std::list<std::string> ui_status_screen_list;
lv_obj_t *status_list = nullptr;
#define MAX_STATUS_SCREEN_LIST 10
void main_screen();
void status_screen();

bool status_list_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Add ||
      action == ESP3DValuesCbAction::Update) {
    esp3d_log("status_list_cb: %d", (int)status_list);
    if (ui_status_screen_list.size() > MAX_STATUS_SCREEN_LIST) {
      ui_status_screen_list.pop_back();
    }
    ui_status_screen_list.push_front(value);
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::status_list) {
      status_screen();
    }
  } else if (action == ESP3DValuesCbAction::Clear) {
    ui_status_screen_list.clear();
  } else {
    return false;
  }
  return true;
}

static void event_handler_button_back(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    esp3d_log("Clicked");
    main_screen();
  }
}
#define STATUS_SCREEN_H_PAD 10
#define STATUS_SCREEN_V_PAD 10
void status_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::status_list);
  // Screen creation
  esp3d_log("Main screen creation");
  lv_obj_t *ui_status_screen = lv_obj_create(NULL);
  const ESP3DValuesDescription *status_bar_desc =
      esp3dTftValues.get_description(ESP3DValuesIndex::status_bar_label);
  // the control is deleted but not the label is not set to null so we need to
  // do it to avoid crash if we try to update it
  if (status_bar_desc != nullptr &&
      ESP3DScreenType::status_list == esp3dTftui.get_current_screen()) {
    status_bar_desc->label = nullptr;
  }
  // Apply background color
  lv_obj_set_style_bg_color(ui_status_screen, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_clear_flag(ui_status_screen, LV_OBJ_FLAG_SCROLLABLE);
  // Fill screen content
  lv_obj_set_style_pad_right(ui_status_screen, STATUS_SCREEN_V_PAD,
                             LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(ui_status_screen, STATUS_SCREEN_H_PAD,
                              LV_PART_MAIN);
  lv_obj_set_style_pad_left(ui_status_screen, STATUS_SCREEN_V_PAD,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_top(ui_status_screen, STATUS_SCREEN_H_PAD, LV_PART_MAIN);

  // TODO: Add your code here

  status_list = lv_list_create(ui_status_screen);
  lv_obj_set_align(status_list, LV_ALIGN_TOP_LEFT);
  for (auto &line : ui_status_screen_list) {
    lv_list_add_btn(status_list, "", line.c_str());
  }

  lv_obj_t *btn_back = lv_btn_create(ui_status_screen);
  lv_obj_t *label_btn_back = lv_label_create(btn_back);
  lv_label_set_text(label_btn_back, LV_SYMBOL_NEW_LINE);
  lv_obj_set_align(label_btn_back, LV_ALIGN_CENTER);
  lv_obj_set_align(btn_back, LV_ALIGN_BOTTOM_RIGHT);
  lv_obj_add_event_cb(btn_back, event_handler_button_back, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_update_layout(btn_back);
  lv_obj_set_height(status_list, LV_VER_RES - (STATUS_SCREEN_V_PAD * 2));
  lv_obj_set_width(status_list, LV_HOR_RES - (STATUS_SCREEN_H_PAD * 3) -
                                    lv_obj_get_width(btn_back));
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_status_screen);
  lv_obj_del(ui_current_screen);
}
