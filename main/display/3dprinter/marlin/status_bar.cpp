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
#include "lvgl.h"
#include "version.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
lv_obj_t *status_label = NULL;
// Create style for status bar
lv_style_t style_status_bar;

#define STATUS_BAR_HEIGHT_RADIUS 10
#define STATUS_BAR_H_PAD 10
#define STATUS_BAR_V_PAD 4

lv_obj_t *status_bar(lv_obj_t *screen, lv_obj_t *page_container) {
  // Create style for status bar
  if (!status_label) {
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
    lv_obj_del(status_label);
    status_label = NULL;
  }
  status_label = lv_label_create(screen);
  // TODO : Use the variable value
  lv_label_set_text(status_label,
                    "ESP3D-TFT " ESP3D_TFT_VERSION " " __DATE__ " " __TIME__);
  lv_obj_set_width(status_label, LV_HOR_RES);
  lv_label_set_long_mode(status_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  // Apply style to status bar
  lv_obj_add_style(status_label, &style_status_bar, LV_PART_MAIN);
  return status_label;
}
