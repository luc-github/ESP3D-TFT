
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

#define CURRENT_MAIN_BG_COLOR lv_color_hex(0x000000)

#define CURRENT_BUTTON_COLOR_PALETTE LV_PALETTE_BLUE
#define CURRENT_BUTTON_COLOR_PALETTE_DARKEN 2
#define CURRENT_BUTTON_RADIUS_VALUE 3
#define CURRENT_BUTTON_BORDER_VALUE 2
#define CURRENT_BUTTON_PAD 10
#define CURRENT_BUTTON_COLOR lv_color_hex(0xFFFFFF)
#define CURRENT_BUTTON_COLOR_PRESSED_OUTLINE 30
#define CURRENT_BUTTON_COLOR_PRESSED_SHADOW_OFFSET 3

// Create styles for main background
lv_style_t style_main_bg;
// Create styles for buttons
lv_style_t style_btn_default;
lv_style_t style_btn_pressed;

bool init_styles() {
  /*
  Main background
  */
  // Create style for button default state
  // lv_style_init(&style_main_bg);

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
  lv_style_set_border_color(&style_btn_default,
                            lv_palette_main(LV_PALETTE_GREY));

  lv_style_set_outline_opa(&style_btn_default, LV_OPA_COVER);
  lv_style_set_outline_color(&style_btn_default,
                             lv_palette_main(CURRENT_BUTTON_COLOR_PALETTE));

  lv_style_set_text_color(&style_btn_default, CURRENT_BUTTON_COLOR);
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

  return true;
}

bool apply_style(lv_obj_t* obj, ESP3DStyleType type) {
  switch (type) {
    case ESP3DStyleType::main_bg:
      lv_obj_add_style(obj, &style_main_bg, LV_STATE_DEFAULT);
      // Apply background color
      lv_obj_set_style_bg_color(obj, CURRENT_MAIN_BG_COLOR, LV_PART_MAIN);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
      break;
    case ESP3DStyleType::button:
      lv_obj_remove_style_all(obj); /*Remove the style coming from the
      // theme*/
      lv_obj_add_style(obj, &style_btn_default, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);
      break;
    default:
      return false;
  }
  return true;
}