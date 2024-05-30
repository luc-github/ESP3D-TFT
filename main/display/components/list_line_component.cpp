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

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "symbol_button_component.h"

// Note: This math works because of the bottom-only border.
//   The top still gets a hidden border that acts like extra padding, pushing
//   the content down. The 3*border_width calculation therefore accounts for the
//   *padding* on the top and bottom, and then the bottom border itself.
#define LIST_LINE_CONTENT_HEIGHT \
  (ESP3D_LIST_LINE_HEIGHT - (3 * ESP3D_LIST_LINE_BORDER_SIZE))

/**********************
 *  Namespace
 **********************/
namespace listLine {
/**
 * @brief Creates a list line component and adds it to the specified container.
 *
 * @param container The container object to which the list line component will
 * be added.
 * @return The created list line component object.
 */
lv_obj_t *create(lv_obj_t *container) {
  lv_obj_t *line_container = lv_obj_create(container);
  if (lv_obj_is_valid(line_container)) {
    lv_obj_update_layout(container);
    lv_obj_set_size(line_container, lv_obj_get_content_width(container),
                    ESP3D_LIST_LINE_HEIGHT);

    lv_obj_set_style_pad_all(line_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(line_container, ESP3D_BUTTON_PRESSED_OUTLINE,
                              LV_PART_MAIN);
    lv_obj_set_style_pad_right(line_container, ESP3D_BUTTON_PRESSED_OUTLINE,
                               LV_PART_MAIN);

    lv_obj_set_style_border_width(line_container, ESP3D_LIST_LINE_BORDER_SIZE,
                                  LV_PART_MAIN);
    lv_obj_set_style_border_side(line_container, LV_BORDER_SIDE_BOTTOM,
                                 LV_PART_MAIN);

    lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(line_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_column(line_container, ESP3D_BUTTON_PRESSED_OUTLINE,
                                LV_PART_MAIN);
  } else {
    esp3d_log_e("Failed to create list line container");
  }
  return line_container;
}

/**
 * Adds a label to the line container.
 *
 * @param lbl The text to be displayed in the label.
 * @param line_container The container object where the label will be added.
 * @param grow A boolean value indicating whether the container should grow to
 * fit the label.
 * @return The created label object.
 */
lv_obj_t *add_label(const char *lbl, lv_obj_t *line_container, bool grow) {
  lv_obj_t *label_txt = lv_label_create(line_container);
  lv_label_set_text(label_txt, lbl);

  lv_obj_update_layout(label_txt);
  size_t label_height = lv_obj_get_height(label_txt);
  size_t pad_v = (LIST_LINE_CONTENT_HEIGHT - label_height) / 2;
  lv_obj_set_style_pad_top(label_txt, pad_v, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(label_txt, pad_v, LV_PART_MAIN);

  if (grow) {
    lv_obj_set_flex_grow(label_txt, 1);
    lv_label_set_long_mode(label_txt, LV_LABEL_LONG_SCROLL_CIRCULAR);
  }
  return label_txt;
}

/**
 * @brief Adds a button to the line container.
 *
 * This function creates and adds a button to the specified line container.
 *
 * @param lbl The label text for the button.
 * @param line_container The line container to which the button will be added.
 *
 * @return The created button object.
 */
lv_obj_t *add_button(const char *lbl, lv_obj_t *line_container) {
  lv_obj_t *button_list =
      symbolButton::create(line_container, lbl, ESP3D_LIST_LINE_BUTTON_WIDTH,
                           LIST_LINE_CONTENT_HEIGHT);
  return button_list;
}
}  // namespace listLine