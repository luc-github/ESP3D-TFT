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
#include "esp3d_styles.h"
#include "esp3d_values.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
lv_obj_t *status_bar_container = NULL;

void status_screen();

static void event_handler_status_list(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    esp3d_log("Clicked");
    status_screen();
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

  status_bar_container = lv_obj_create(screen);
  status_bar_desc->label = lv_label_create(status_bar_container);
  apply_style(status_bar_desc->label, ESP3DStyleType::status_bar);
  lv_label_set_text(status_bar_desc->label, status_bar_desc->value.c_str());

  // Apply style to status bar

  lv_obj_remove_style_all(status_bar_container);
  lv_obj_set_width(status_bar_container, LV_HOR_RES);
  lv_obj_set_height(status_bar_container, LV_SIZE_CONTENT);
  lv_obj_add_event_cb(status_bar_container, event_handler_status_list,
                      LV_EVENT_CLICKED, NULL);
  return (status_bar_container);
}
