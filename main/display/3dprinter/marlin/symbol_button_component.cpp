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

lv_obj_t *create_symbol_button(lv_obj_t *container, const char *text,
                               int width = SYMBOL_BUTTON_WIDTH,
                               int height = SYMBOL_BUTTON_HEIGHT,
                               bool center = true, bool slash = false,
                               int rotation = 0) {
  lv_obj_t *btn = lv_btn_create(container);
  apply_style(btn, ESP3DStyleType::button);
  if (width != -1) lv_obj_set_width(btn, width);
  if (height != -1) lv_obj_set_height(btn, height);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, text);
  lv_obj_update_layout(label);

  int32_t label_width = lv_obj_get_width(label);
  int32_t label_height = lv_obj_get_height(label);

  lv_obj_center(label);
  if (center) lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

  if (rotation != 0) {
    lv_obj_set_style_transform_pivot_x(label, label_width / 2, 0);
    lv_obj_set_style_transform_pivot_y(label, label_height / 2, 0);
    lv_obj_set_style_transform_angle(label, rotation * 10, 0);
    lv_obj_center(label);
  }

  if (slash) {
    lv_obj_t *label2 = lv_label_create(btn);
    lv_label_set_text(label2, LV_SYMBOL_SLASH);
    lv_obj_center(label2);
  }
  return btn;
}
