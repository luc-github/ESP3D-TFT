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

#include "list_line_component.h"

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "symbol_button_component.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace listLine {
lv_obj_t *create_list_line_container(lv_obj_t *container) {
  lv_obj_t *line_container = lv_obj_create(container);
  if (line_container) {
    lv_obj_update_layout(container);
    lv_obj_set_width(line_container, lv_obj_get_content_width(container));
    lv_obj_set_height(line_container, LIST_LINE_HEIGHT);

    lv_obj_set_style_pad_all(line_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(line_container, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_right(line_container, CURRENT_BUTTON_PRESSED_OUTLINE,
                               LV_PART_MAIN);
    lv_obj_set_style_border_side(line_container, LV_BORDER_SIDE_BOTTOM,
                                 LV_PART_MAIN);
    lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(line_container, LV_OBJ_FLAG_SCROLLABLE);
  }
  return line_container;
}

lv_obj_t *add_label_to_line(const char *lbl, lv_obj_t *line_container,
                            bool grow) {
  lv_obj_t *label_txt = lv_label_create(line_container);
  lv_label_set_text(label_txt, lbl);
  lv_obj_update_layout(label_txt);
  size_t header_height = lv_obj_get_height(label_txt);
  size_t pad_top = (LIST_LINE_HEIGHT - header_height) / 2;
  lv_obj_set_style_pad_top(label_txt, pad_top, LV_PART_MAIN);
  if (grow) {
    lv_obj_set_flex_grow(label_txt, 1);
    lv_label_set_long_mode(label_txt, LV_LABEL_LONG_SCROLL_CIRCULAR);
  }
  return label_txt;
}

lv_obj_t *add_button_to_line(const char *lbl, lv_obj_t *line_container) {
  lv_obj_t *button_list = symbolButton::create_symbol_button(
      line_container, lbl, LIST_LINE_BUTTON_WIDTH, -1);
  return button_list;
}
}  // namespace listLine