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

#include "status_screen.h"

#include <lvgl.h>

#include <list>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_values.h"
#include "main_screen.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace statusScreen {
static std::list<std::string> ui_status_screen_list;

#define MAX_STATUS_SCREEN_LIST 10

bool status_list_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Clear) {
    ui_status_screen_list.clear();
    esp3d_log("status_list clear");
  } else if (action == ESP3DValuesCbAction::Add ||
             action == ESP3DValuesCbAction::Update) {
    if (!value) {
      esp3d_log_e("status_list_cb value is null");
      return false;
    }
    esp3d_log("status_list_cb size is %d", ui_status_screen_list.size());
    if (ui_status_screen_list.size() > MAX_STATUS_SCREEN_LIST) {
      esp3d_log("status_list size %d pop", ui_status_screen_list.size());
      ui_status_screen_list.pop_back();
    }
    std::string tmp = value;
    ui_status_screen_list.push_front(tmp);

    esp3d_log("status_list pushing %s now size is %d", value,
              ui_status_screen_list.size());
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::status_list) {
      esp3d_log("Refreshing status screen");
      status_screen();
    } else {
      esp3d_log("No refresh needed");
    }
  } else {
    esp3d_log_e("status_list unknown action %d", (int)action);
    return false;
  }
  return true;
}

static void event_handler_button_back(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    esp3d_log("Clicked");
    mainScreen::main_screen();
  }
}

void status_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  lv_obj_t *ui_status_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_status_screen);
  lv_obj_del(ui_current_screen);
  // Apply background color
  apply_style(ui_status_screen, ESP3DStyleType::main_bg);
  // Create screen container
  lv_obj_t *ui_status_screen_container = lv_obj_create(ui_status_screen);
  apply_style(ui_status_screen_container, ESP3DStyleType::col_container);
  lv_obj_set_size(ui_status_screen_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(ui_status_screen_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_top(ui_status_screen_container, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(ui_status_screen_container, 0, LV_PART_MAIN);

  lv_obj_t *ui_status_list_ctl = lv_list_create(ui_status_screen_container);
  apply_style(ui_status_list_ctl, ESP3DStyleType::status_list);
  lv_obj_set_align(ui_status_list_ctl, LV_ALIGN_TOP_MID);
  esp3d_log("status_list size is %d", ui_status_screen_list.size());
  for (auto &line : ui_status_screen_list) {
    lv_list_add_btn(ui_status_list_ctl, "", line.c_str());
  }

  lv_obj_t *btn_back = lv_btn_create(ui_status_screen_container);
  apply_style(btn_back, ESP3DStyleType::embedded_button);
  lv_obj_set_width(btn_back, LV_HOR_RES);
  lv_obj_t *label_btn_back = lv_label_create(btn_back);
  lv_label_set_text(label_btn_back, LV_SYMBOL_UP);
  lv_obj_set_align(label_btn_back, LV_ALIGN_CENTER);
  lv_obj_set_align(btn_back, LV_ALIGN_BOTTOM_MID);
  lv_obj_add_event_cb(btn_back, event_handler_button_back, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_update_layout(btn_back);
  lv_obj_set_height(ui_status_list_ctl,
                    LV_VER_RES - lv_obj_get_height(btn_back));
  lv_obj_set_width(ui_status_list_ctl, LV_HOR_RES);
  esp3dTftui.set_current_screen(ESP3DScreenType::status_list);
}
}  // namespace statusScreen