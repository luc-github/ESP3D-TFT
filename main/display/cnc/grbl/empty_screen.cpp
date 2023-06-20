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

/**********************
 *  STATIC PROTOTYPES
 **********************/

void empty_screen() {
  // Screen creation
  esp3d_log("Main screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Apply background color
  lv_obj_set_style_bg_color(ui_new_screen, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_clear_flag(ui_new_screen, LV_OBJ_FLAG_SCROLLABLE);
  // Fill screen content

  // TODO: Add your code here

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
}
