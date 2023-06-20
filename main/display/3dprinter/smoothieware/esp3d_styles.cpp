
/*
  esp3d_values.cpp -  values esp3d functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "esp3d_styles.h"

#include "esp3d_log.h"
#include "esp3d_styles_res.h"

#define CURRENT_MAIN_BG_COLOR lv_color_hex(0x000000)

#define CURRENT_BG_LABEL_TEXT_COLOR lv_color_hex(0xFFFFFF)

#define CURRENT_STATUS_BAR_TEXT_COLOR lv_color_hex(0x000000)
#define CURRENT_STATUS_BAR_BG_COLOR lv_color_hex(0xFFFFFF)

#define CURRENT_STATUS_BAR_BORDER_COLOR lv_palette_main(LV_PALETTE_BLUE)

#define CURRENT_BUTTON_COLOR_PALETTE LV_PALETTE_BLUE
#define CURRENT_BUTTON_COLOR_PALETTE_DARKEN 2

#define CURRENT_BUTTON_BORDER_COLOR lv_palette_main(LV_PALETTE_GREY)
#define CURRENT_BUTTON_TEXT_COLOR lv_color_hex(0xFFFFFF)

// Create styles for main background
lv_style_t style_main_bg;
lv_style_t style_bg_label;

// Create styles for status bar
lv_style_t style_status_bar_default;

lv_style_t style_status_list_default;

// Create styles for buttons
lv_style_t style_btn_default;
lv_style_t style_btn_pressed;

lv_style_t style_embedded_btn_default;
lv_style_t style_embedded_btn_pressed;

// Create styles for containers
lv_style_t style_col_container_default;
lv_style_t style_row_container_default;

bool init_styles() {
  /*
  Main background
  */
  // Create style for main background
  // N/A

  /*
  Label on main background
  */
  lv_style_init(&style_bg_label);

  lv_style_set_text_opa(&style_bg_label, LV_OPA_COVER);
  lv_style_set_text_color(&style_bg_label, CURRENT_BG_LABEL_TEXT_COLOR);
  lv_style_set_bg_color(&style_bg_label, CURRENT_MAIN_BG_COLOR);

  /*
  Status bar
  */
  lv_style_init(&style_status_bar_default);

  lv_style_set_text_opa(&style_status_bar_default, LV_OPA_COVER);
  lv_style_set_text_color(&style_status_bar_default,
                          CURRENT_STATUS_BAR_TEXT_COLOR);
  lv_style_set_bg_opa(&style_status_bar_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_status_bar_default, CURRENT_STATUS_BAR_BG_COLOR);
  lv_style_set_radius(&style_status_bar_default, CURRENT_STATUS_BAR_RADIUS);
  lv_style_set_pad_hor(&style_status_bar_default, CURRENT_STATUS_BAR_H_PAD);
  lv_style_set_pad_ver(&style_status_bar_default, CURRENT_STATUS_BAR_V_PAD);
  lv_style_set_border_width(&style_status_bar_default,
                            CURRENT_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_status_bar_default,
                            CURRENT_STATUS_BAR_BORDER_COLOR);

  /*
   Buttons
  */
  // Create style for button default state
  lv_style_init(&style_btn_default);

  lv_style_set_radius(&style_btn_default, CURRENT_BUTTON_RADIUS_VALUE);
  lv_style_set_bg_opa(&style_btn_default, LV_OPA_100);
  lv_style_set_bg_color(&style_btn_default,
                        lv_palette_main(CURRENT_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_btn_default,
      lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                        CURRENT_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_btn_default, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style_btn_default, LV_OPA_40);
  lv_style_set_border_width(&style_btn_default, CURRENT_BUTTON_BORDER_VALUE);
  lv_style_set_border_color(&style_btn_default, CURRENT_BUTTON_BORDER_COLOR);

  lv_style_set_outline_opa(&style_btn_default, LV_OPA_COVER);
  lv_style_set_outline_color(&style_btn_default,
                             lv_palette_main(CURRENT_BUTTON_COLOR_PALETTE));

  lv_style_set_text_color(&style_btn_default, CURRENT_BUTTON_TEXT_COLOR);
  lv_style_set_pad_all(&style_btn_default, CURRENT_BUTTON_PAD);

  // Create style for button pressed state
  lv_style_init(&style_btn_pressed);

  /*Add a large outline when pressed*/
  lv_style_set_outline_width(&style_btn_pressed,
                             CURRENT_BUTTON_COLOR_PRESSED_OUTLINE);
  lv_style_set_outline_opa(&style_btn_pressed, LV_OPA_TRANSP);

  lv_style_set_shadow_ofs_y(&style_btn_pressed,
                            CURRENT_BUTTON_COLOR_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_btn_pressed,
                        lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                                          CURRENT_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_color(
      &style_btn_pressed,
      lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                        CURRENT_BUTTON_COLOR_PALETTE_DARKEN * 2));
  /*Add a transition to the outline*/
  static lv_style_transition_dsc_t trans;
  static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH,
                                    LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
  lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0,
                               NULL);
  lv_style_set_transition(&style_btn_pressed, &trans);

  /*
  Embedded buttons
  */
  // Create style for embedded button default state
  lv_style_init(&style_embedded_btn_default);

  lv_style_set_radius(&style_embedded_btn_default, 0);
  lv_style_set_bg_opa(&style_embedded_btn_default, LV_OPA_100);
  lv_style_set_bg_color(&style_embedded_btn_default,
                        lv_palette_main(CURRENT_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_embedded_btn_default,
      lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                        CURRENT_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_embedded_btn_default, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style_embedded_btn_default, LV_OPA_40);
  lv_style_set_border_width(&style_embedded_btn_default,
                            CURRENT_BUTTON_BORDER_VALUE);
  lv_style_set_border_color(&style_embedded_btn_default,
                            CURRENT_BUTTON_BORDER_COLOR);

  lv_style_set_outline_opa(&style_embedded_btn_default, LV_OPA_COVER);
  lv_style_set_outline_color(&style_embedded_btn_default,
                             lv_palette_main(CURRENT_BUTTON_COLOR_PALETTE));

  lv_style_set_text_color(&style_embedded_btn_default,
                          CURRENT_BUTTON_TEXT_COLOR);
  lv_style_set_pad_all(&style_embedded_btn_default, CURRENT_BUTTON_PAD);

  // Create style for embedded button pressed state
  lv_style_init(&style_embedded_btn_pressed);

  lv_style_set_shadow_ofs_y(&style_embedded_btn_pressed,
                            CURRENT_BUTTON_COLOR_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_embedded_btn_pressed,
                        lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                                          CURRENT_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_color(
      &style_embedded_btn_pressed,
      lv_palette_darken(CURRENT_BUTTON_COLOR_PALETTE,
                        CURRENT_BUTTON_COLOR_PALETTE_DARKEN * 2));

  /*
  Status list
  */
  lv_style_init(&style_status_list_default);
  lv_style_set_text_opa(&style_status_list_default, LV_OPA_COVER);
  lv_style_set_text_color(&style_status_list_default,
                          CURRENT_STATUS_BAR_TEXT_COLOR);
  lv_style_set_bg_opa(&style_status_list_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_status_list_default,
                        CURRENT_STATUS_BAR_BG_COLOR);
  lv_style_set_radius(&style_status_list_default, 0);
  // lv_style_set_pad_hor(&style_status_list_default, CURRENT_STATUS_BAR_H_PAD);
  // lv_style_set_pad_ver(&style_status_list_default, CURRENT_STATUS_BAR_V_PAD);
  lv_style_set_border_width(&style_status_list_default,
                            CURRENT_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_status_list_default,
                            CURRENT_STATUS_BAR_BORDER_COLOR);
  /*
  Col Container
  */
  lv_style_init(&style_col_container_default);
  lv_style_set_radius(&style_col_container_default, CURRENT_CONTAINER_RADIUS);
  lv_style_set_layout(&style_col_container_default, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_col_container_default, LV_FLEX_FLOW_COLUMN);
  lv_style_set_flex_main_place(&style_col_container_default,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  /*
  Row Container
  */
  lv_style_init(&style_row_container_default);
  lv_style_set_radius(&style_row_container_default, CURRENT_CONTAINER_RADIUS);
  lv_style_set_layout(&style_row_container_default, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_row_container_default, LV_FLEX_FLOW_ROW);
  lv_style_set_flex_main_place(&style_row_container_default,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  return true;
}

bool apply_style(lv_obj_t* obj, ESP3DStyleType type) {
  if (type != ESP3DStyleType::main_bg) {
    lv_obj_remove_style_all(obj); /*Remove the style coming from the
    // theme*/
  }
  switch (type) {
    case ESP3DStyleType::main_bg:
      lv_obj_add_style(obj, &style_main_bg, LV_STATE_DEFAULT);
      // Apply background color
      lv_obj_set_style_bg_color(obj, CURRENT_MAIN_BG_COLOR, LV_PART_MAIN);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
      break;
    case ESP3DStyleType::bg_label:
      lv_obj_add_style(obj, &style_bg_label, LV_STATE_DEFAULT);
      break;
    case ESP3DStyleType::status_bar:
      lv_obj_add_style(obj, &style_status_bar_default, LV_STATE_DEFAULT);
      lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_obj_set_width(obj, LV_HOR_RES);
      lv_obj_set_align(obj, LV_ALIGN_CENTER);

      break;
    case ESP3DStyleType::button:
      lv_obj_add_style(obj, &style_btn_default, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);
      break;

    case ESP3DStyleType::embedded_button:
      lv_obj_add_style(obj, &style_embedded_btn_default, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_embedded_btn_pressed, LV_STATE_PRESSED);
      break;

    case ESP3DStyleType::row_container:
      lv_obj_add_style(obj, &style_row_container_default, LV_STATE_DEFAULT);
      lv_obj_set_style_clip_corner(obj, true, 0);
      break;

    case ESP3DStyleType::col_container:
      lv_obj_add_style(obj, &style_col_container_default, LV_STATE_DEFAULT);
      lv_obj_set_style_clip_corner(obj, true, 0);
      break;

    case ESP3DStyleType::status_list:
      lv_obj_add_style(obj, &style_status_list_default, LV_STATE_DEFAULT);
      break;

    default:
      return false;
  }
  return true;
}