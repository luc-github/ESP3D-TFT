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
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back) {
  lv_obj_t *ui_container = lv_obj_create(parent);
  apply_style(ui_container, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_update_layout(button_back);
  lv_obj_set_size(ui_container, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(button_back) -
                      (1 * CURRENT_BUTTON_PRESSED_OUTLINE));
  lv_obj_set_style_pad_top(ui_container, CURRENT_BUTTON_PRESSED_OUTLINE * 1,
                           LV_PART_MAIN);
  return ui_container;
}
