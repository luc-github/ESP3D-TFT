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

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_values.h"
#include "lvgl.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
lv_obj_t *status_bar_container = NULL;
//  Create style for status bar
lv_style_t style_status_bar;

#define STATUS_BAR_HEIGHT_RADIUS 10
#define STATUS_BAR_H_PAD 10
#define STATUS_BAR_V_PAD 4

static void event_handler_status_list(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    esp3d_log("Clicked");
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    esp3d_log("Toggled");
  }
}

lv_obj_t *status_bar(lv_obj_t *screen, lv_obj_t *page_container) {
  // Create style for status bar
  const ESP3DValuesDescription *status_bar_desc =
      esp3dTftValues.get_description(ESP3DValuesIndex::status_bar_label);
  if (status_bar_desc == nullptr) {
    esp3d_log_e("status_bar: description is null cancel");
    return nullptr;
  }
  if (!status_bar_desc->label) {
    esp3d_log("status_bar: label is null, create style");
    lv_style_init(&style_status_bar);
    lv_style_set_text_opa(&style_status_bar, LV_OPA_COVER);
    lv_style_set_text_color(&style_status_bar, lv_color_hex(0x000000));
    lv_style_set_bg_opa(&style_status_bar, LV_OPA_COVER);
    lv_style_set_bg_color(&style_status_bar, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_status_bar, STATUS_BAR_HEIGHT_RADIUS);
    lv_style_set_pad_hor(&style_status_bar, STATUS_BAR_H_PAD);
    lv_style_set_pad_ver(&style_status_bar, STATUS_BAR_V_PAD);
  } else  // Create status bar object
  {
    esp3d_log("status_bar: label is not null, delete it");
    lv_obj_del(status_bar_desc->label);
    status_bar_desc->label = NULL;
    lv_obj_del(status_bar_container);
    status_bar_container = NULL;
  }
  status_bar_container = lv_obj_create(screen);
  status_bar_desc->label = lv_label_create(status_bar_container);
  lv_label_set_text(status_bar_desc->label, status_bar_desc->value.c_str());
  lv_label_set_long_mode(status_bar_desc->label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  // Apply style to status bar
  lv_obj_add_style(status_bar_desc->label, &style_status_bar, LV_PART_MAIN);
  lv_obj_remove_style_all(status_bar_container);
  lv_obj_set_width(status_bar_desc->label, LV_HOR_RES);
  lv_obj_set_align(status_bar_desc->label, LV_ALIGN_CENTER);
  lv_obj_set_width(status_bar_container, LV_HOR_RES);
  lv_obj_set_height(status_bar_container, LV_SIZE_CONTENT);
  lv_obj_add_event_cb(status_bar_container, event_handler_status_list,
                      LV_EVENT_CLICKED, NULL);
  return (status_bar_container);
}
