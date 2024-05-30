
/*
  esp3d_styles.cpp -  values esp3d functions class

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
namespace ESP3DStyle {
// Create styles for main background
lv_style_t style_main_bg;
lv_style_t style_bg_label;
lv_style_t style_read_only_value;
lv_style_t style_read_only_setting;
lv_style_t style_scrollbar_default;
lv_style_t style_scrollbar_active;

// Create styles for status bar
lv_style_t style_status_bar_default;
lv_style_t style_status_list_default;

// Create styles for progression area
lv_style_t style_progression_area;

// Create styles for buttons
lv_style_t style_btn_default;
lv_style_t style_btn_pressed;

lv_style_t style_btn_radio_default;
lv_style_t style_btn_radio_pressed;
lv_style_t style_btn_radio_checked;

lv_style_t style_btn_matrix_default;
lv_style_t style_btn_matrix_pressed;
lv_style_t style_btn_matrix_checked;
lv_style_t style_btn_matrix_bar;

lv_style_t style_btn_msb_box_default;
lv_style_t style_btn_msb_box_pressed;

lv_style_t style_embedded_btn_default;
lv_style_t style_embedded_btn_pressed;

// Create styles for containers
lv_style_t style_col_container_default;
lv_style_t style_row_container_default;
lv_style_t style_simple_container_default;
lv_style_t style_text_container;
lv_style_t style_spinner_screen;
lv_style_t style_spinner_text;

bool init() {
  /*
  Spinner screen
  */
  lv_style_init(&style_spinner_screen);
  lv_style_set_text_opa(&style_spinner_screen, LV_OPA_COVER);
  lv_style_set_text_color(&style_spinner_screen,
                          ESP3D_SCREEN_BACKGROUND_TEXT_COLOR);
  lv_style_set_bg_color(&style_spinner_screen, ESP3D_SCREEN_BACKGROUND_COLOR);
  lv_style_set_bg_opa(&style_spinner_screen, LV_OPA_50);
  /*
  Spinner text
  */
  lv_style_init(&style_spinner_text);
  lv_style_set_text_opa(&style_spinner_text, LV_OPA_COVER);
  lv_style_set_text_color(&style_spinner_text,
                          ESP3D_SCREEN_BACKGROUND_TEXT_COLOR);

  /*
  Main background
  */
  lv_style_init(&style_main_bg);
  lv_style_set_text_opa(&style_main_bg, LV_OPA_COVER);
  lv_style_set_text_color(&style_main_bg, ESP3D_SCREEN_BACKGROUND_TEXT_COLOR);
  lv_style_set_bg_color(&style_main_bg, ESP3D_SCREEN_BACKGROUND_COLOR);

  /*
  Label on main background
  */
  lv_style_init(&style_bg_label);
  lv_style_set_text_opa(&style_bg_label, LV_OPA_COVER);
  lv_style_set_text_color(&style_bg_label, ESP3D_SCREEN_BACKGROUND_TEXT_COLOR);
  lv_style_set_bg_color(&style_bg_label, ESP3D_SCREEN_BACKGROUND_COLOR);

  /*
  read only value on main background
  */
  lv_style_init(&style_read_only_value);
  lv_style_set_text_opa(&style_read_only_value, LV_OPA_COVER);
  lv_style_set_text_color(&style_read_only_value,
                          ESP3D_SCREEN_BACKGROUND_TEXT_COLOR);
  lv_style_set_bg_color(&style_read_only_value, ESP3D_SCREEN_BACKGROUND_COLOR);
  lv_style_set_radius(&style_read_only_value, ESP3D_STATUS_BAR_RADIUS);
  lv_style_set_border_width(&style_read_only_value,
                            ESP3D_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_read_only_value,
                            ESP3D_STATUS_BAR_BORDER_COLOR);
  lv_style_set_text_align(&style_read_only_value, LV_TEXT_ALIGN_CENTER);
  lv_style_set_pad_top(&style_read_only_value, ESP3D_BUTTON_PAD);
  lv_style_set_pad_bottom(&style_read_only_value, ESP3D_BUTTON_PAD);

  /*
  read only setting on main background
  */
  lv_style_init(&style_read_only_setting);
  lv_style_set_text_opa(&style_read_only_setting, LV_OPA_COVER);
  lv_style_set_text_color(&style_read_only_setting,
                          ESP3D_BUTTON_PRESSED_TEXT_COLOR);
  lv_style_set_bg_color(&style_read_only_setting,
                        ESP3D_SCREEN_BACKGROUND_COLOR);
  lv_style_set_radius(&style_read_only_setting, ESP3D_STATUS_BAR_RADIUS);
  lv_style_set_border_width(&style_read_only_setting,
                            ESP3D_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_read_only_setting,
                            ESP3D_STATUS_BAR_BORDER_COLOR);
  lv_style_set_text_align(&style_read_only_setting, LV_TEXT_ALIGN_CENTER);
  lv_style_set_pad_top(&style_read_only_setting, ESP3D_BUTTON_PAD);
  lv_style_set_pad_bottom(&style_read_only_setting, ESP3D_BUTTON_PAD);

  /*
  Status bar
  */
  lv_style_init(&style_status_bar_default);

  lv_style_set_text_opa(&style_status_bar_default, LV_OPA_COVER);
  lv_style_set_text_color(&style_status_bar_default,
                          ESP3D_STATUS_BAR_TEXT_COLOR);
  lv_style_set_bg_opa(&style_status_bar_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_status_bar_default, ESP3D_STATUS_BAR_BG_COLOR);
  lv_style_set_radius(&style_status_bar_default, ESP3D_STATUS_BAR_RADIUS);
  lv_style_set_pad_hor(&style_status_bar_default, ESP3D_STATUS_BAR_H_PAD);
  lv_style_set_pad_ver(&style_status_bar_default, ESP3D_STATUS_BAR_V_PAD);
  lv_style_set_border_width(&style_status_bar_default,
                            ESP3D_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_status_bar_default,
                            ESP3D_STATUS_BAR_BORDER_COLOR);
  /*
    Progression area
  */

  lv_style_init(&style_progression_area);

  lv_style_set_text_opa(&style_progression_area, LV_OPA_COVER);
  lv_style_set_text_color(&style_progression_area,
                          ESP3D_PROGRESSION_AREA_TEXT_COLOR);
  lv_style_set_bg_opa(&style_progression_area, LV_OPA_COVER);
  lv_style_set_bg_color(&style_progression_area,
                        ESP3D_PROGRESSION_AREA_BG_COLOR);
  lv_style_set_radius(&style_progression_area, ESP3D_STATUS_BAR_RADIUS);
  lv_style_set_pad_hor(&style_progression_area, ESP3D_STATUS_BAR_H_PAD);
  lv_style_set_pad_ver(&style_progression_area, ESP3D_STATUS_BAR_V_PAD);
  lv_style_set_border_width(&style_progression_area,
                            ESP3D_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_progression_area,
                            ESP3D_PROGRESSION_AREA_BORDER_COLOR);
  /*
   Scrollbar
  */

  // Create style for scrollbar default state
  lv_style_init(&style_scrollbar_default);
  lv_style_set_width(&style_scrollbar_default, ESP3D_SCROLL_BAR_WIDTH);
  lv_style_set_bg_opa(&style_scrollbar_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_scrollbar_default,
                        lv_palette_main(LV_PALETTE_GREY));

  lv_style_set_border_color(
      &style_scrollbar_default,
      lv_palette_darken(LV_PALETTE_GREY,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));

  lv_style_set_border_opa(&style_scrollbar_default, LV_OPA_COVER);
  lv_style_set_border_width(&style_scrollbar_default, ESP3D_BUTTON_BORDER_SIZE);
  lv_style_set_radius(&style_scrollbar_default, ESP3D_SCROLL_BAR_RADIUS);
  lv_style_set_pad_all(&style_scrollbar_default, 8);
  // Create style for scrollbar active state
  lv_style_init(&style_scrollbar_active);
  lv_style_set_bg_opa(&style_scrollbar_active, LV_OPA_COVER);
  lv_style_set_bg_color(&style_scrollbar_active,
                        lv_palette_main(LV_PALETTE_GREEN));
  lv_style_set_shadow_width(&style_scrollbar_active,
                            ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_style_set_shadow_color(&style_scrollbar_active,
                            lv_palette_main(LV_PALETTE_GREEN));
  lv_style_set_shadow_spread(&style_scrollbar_active,
                             ESP3D_BUTTON_PRESSED_SHADOW_OFFSET);

  /*
   Radio Buttons
  */
  // Create style for button default state
  lv_style_init(&style_btn_radio_default);
  lv_style_set_radius(&style_btn_radio_default, LV_RADIUS_CIRCLE);
  lv_style_set_border_color(
      &style_btn_radio_default,
      lv_palette_darken(LV_PALETTE_GREY,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));

  // Create style for button pressed state
  lv_style_init(&style_btn_radio_pressed);
  lv_style_set_shadow_width(&style_btn_radio_pressed,
                            ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_style_set_shadow_color(&style_btn_radio_pressed,
                            lv_palette_main(LV_PALETTE_GREEN));
  lv_style_set_shadow_spread(&style_btn_radio_pressed,
                             ESP3D_BUTTON_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_btn_radio_pressed,
                        lv_palette_main(LV_PALETTE_GREEN));

  // Create style for button checked state
  lv_style_init(&style_btn_radio_checked);
  lv_style_set_bg_img_src(&style_btn_radio_checked, NULL);
  lv_style_set_bg_color(&style_btn_radio_checked,
                        lv_palette_main(LV_PALETTE_GREEN));

  /*
   Buttons
  */
  // Create style for button default state
  lv_style_init(&style_btn_default);

  lv_style_set_radius(&style_btn_default, ESP3D_BUTTON_RADIUS);
  lv_style_set_bg_opa(&style_btn_default, LV_OPA_100);
  lv_style_set_bg_color(&style_btn_default,
                        lv_palette_main(ESP3D_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_btn_default, lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                                            ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_btn_default, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style_btn_default, LV_OPA_40);
  lv_style_set_border_width(&style_btn_default, ESP3D_BUTTON_BORDER_SIZE);
  lv_style_set_border_color(&style_btn_default, ESP3D_BUTTON_BORDER_COLOR);

  lv_style_set_outline_opa(&style_btn_default, LV_OPA_COVER);
  lv_style_set_outline_color(
      &style_btn_default, lv_palette_main(ESP3D_BUTTON_OUTLINE_COLOR_PALETTE));

  lv_style_set_text_color(&style_btn_default, ESP3D_BUTTON_TEXT_COLOR);
  lv_style_set_pad_all(&style_btn_default, ESP3D_BUTTON_PAD);

  // Create style for button pressed state
  lv_style_init(&style_btn_pressed);

  /*Add a large outline when pressed*/
  lv_style_set_outline_width(&style_btn_pressed, ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_style_set_outline_opa(&style_btn_pressed, LV_OPA_TRANSP);

  lv_style_set_shadow_ofs_y(&style_btn_pressed,
                            ESP3D_BUTTON_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_btn_pressed,
                        lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                                          ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_color(
      &style_btn_pressed,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));

  lv_style_set_text_color(&style_btn_pressed, ESP3D_BUTTON_PRESSED_TEXT_COLOR);
  lv_style_set_border_color(&style_btn_pressed,
                            ESP3D_BUTTON_PRESSED_BORDER_COLOR);
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    /*Add a transition to the outline*/
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH,
                                      LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear,
                                 ESP3D_BUTTON_ANIMATION_DELAY, 0, NULL);
    lv_style_set_transition(&style_btn_pressed, &trans);
  }

  /*
  Embedded buttons
  */
  // Create style for embedded button default state
  lv_style_init(&style_embedded_btn_default);

  lv_style_set_radius(&style_embedded_btn_default, 0);
  lv_style_set_bg_opa(&style_embedded_btn_default, LV_OPA_100);
  lv_style_set_bg_color(&style_embedded_btn_default,
                        lv_palette_main(ESP3D_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_embedded_btn_default,
      lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_embedded_btn_default, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style_embedded_btn_default, LV_OPA_40);
  lv_style_set_border_width(&style_embedded_btn_default,
                            ESP3D_BUTTON_BORDER_SIZE);
  lv_style_set_border_color(&style_embedded_btn_default,
                            ESP3D_BUTTON_BORDER_COLOR);

  lv_style_set_outline_opa(&style_embedded_btn_default, LV_OPA_COVER);
  lv_style_set_outline_color(
      &style_embedded_btn_default,
      lv_palette_main(ESP3D_BUTTON_OUTLINE_COLOR_PALETTE));

  lv_style_set_text_color(&style_embedded_btn_default, ESP3D_BUTTON_TEXT_COLOR);
  lv_style_set_pad_all(&style_embedded_btn_default, ESP3D_BUTTON_PAD);

  // Create style for embedded button pressed state
  lv_style_init(&style_embedded_btn_pressed);

  lv_style_set_shadow_ofs_y(&style_embedded_btn_pressed,
                            ESP3D_BUTTON_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_embedded_btn_pressed,
                        lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                                          ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_color(
      &style_embedded_btn_pressed,
      lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));
  lv_style_set_text_color(&style_embedded_btn_pressed,
                          ESP3D_BUTTON_PRESSED_TEXT_COLOR);
  lv_style_set_border_color(&style_embedded_btn_pressed,
                            ESP3D_BUTTON_PRESSED_BORDER_COLOR);

  /*
  Buttons matrix
  */

  // Bar
  lv_style_init(&style_btn_matrix_bar);
  lv_style_set_pad_all(&style_btn_matrix_bar, 1);
  lv_style_set_pad_gap(&style_btn_matrix_bar, 0);
  lv_style_set_clip_corner(&style_btn_matrix_bar, true);
  lv_style_set_radius(&style_btn_matrix_bar, ESP3D_BUTTON_RADIUS);
  lv_style_set_pad_left(&style_btn_matrix_bar, 4);
  lv_style_set_pad_top(&style_btn_matrix_bar, 4);

  // border
  lv_style_set_border_opa(&style_btn_matrix_bar, LV_OPA_40);
  lv_style_set_border_width(&style_btn_matrix_bar, ESP3D_BUTTON_BORDER_SIZE);
  lv_style_set_border_color(&style_btn_matrix_bar, ESP3D_BUTTON_BORDER_COLOR);

  // bg
  lv_style_set_bg_opa(&style_btn_matrix_bar, LV_OPA_100);
  lv_style_set_bg_color(&style_btn_matrix_bar,
                        lv_palette_main(ESP3D_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_btn_matrix_bar,
      lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_btn_matrix_bar, LV_GRAD_DIR_VER);

  // text
  lv_style_set_text_color(&style_btn_matrix_bar, ESP3D_BUTTON_TEXT_COLOR);

  //*****************************************************************

  // Default
  lv_style_init(&style_btn_matrix_default);
  lv_style_set_radius(&style_btn_matrix_default, ESP3D_BUTTON_RADIUS);

  // bg
  // text
  // border

  //********************************************************************

  // Pressed
  lv_style_init(&style_btn_matrix_pressed);
  lv_style_set_pad_all(&style_btn_matrix_pressed, 10);
  lv_style_set_radius(&style_btn_matrix_pressed, ESP3D_BUTTON_RADIUS);
  lv_style_set_outline_opa(&style_btn_matrix_pressed, LV_OPA_40);
  lv_style_set_outline_color(
      &style_btn_matrix_pressed,
      lv_palette_main(ESP3D_BUTTON_OUTLINE_COLOR_PALETTE));
  lv_style_set_outline_width(&style_btn_matrix_pressed, 1);

  // bg
  lv_style_set_bg_opa(&style_btn_matrix_pressed, LV_OPA_60);
  lv_style_set_bg_color(
      &style_btn_matrix_pressed,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));
  lv_style_set_bg_grad_color(
      &style_btn_matrix_pressed,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));
  // text
  lv_style_set_text_color(&style_btn_matrix_pressed,
                          lv_palette_darken(ESP3D_BUTTON_OUTLINE_COLOR_PALETTE,
                                            ESP3D_BUTTON_COLOR_PALETTE_DARKEN));

  // border
  lv_style_set_border_color(&style_btn_matrix_pressed,
                            ESP3D_BUTTON_PRESSED_BORDER_COLOR);
  lv_style_set_border_width(&style_btn_matrix_pressed, 3);
  lv_style_set_border_opa(&style_btn_matrix_pressed, LV_OPA_60);

  //********************************************************************
  // checked
  lv_style_init(&style_btn_matrix_checked);
  lv_style_set_pad_all(&style_btn_matrix_checked, 10);
  lv_style_set_radius(&style_btn_matrix_checked, ESP3D_BUTTON_RADIUS);

  // bg
  lv_style_set_bg_opa(&style_btn_matrix_checked, LV_OPA_40);
  lv_style_set_bg_color(
      &style_btn_matrix_checked,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));
  lv_style_set_bg_grad_color(
      &style_btn_matrix_checked,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));
  // text
  lv_style_set_text_color(&style_btn_matrix_checked,
                          ESP3D_BUTTON_PRESSED_TEXT_COLOR);
  // border
  lv_style_set_border_color(&style_btn_matrix_checked,
                            ESP3D_BUTTON_PRESSED_BORDER_COLOR);
  lv_style_set_border_width(&style_btn_matrix_checked, 2);
  lv_style_set_border_opa(&style_btn_matrix_checked, LV_OPA_40);

  /*
  Buttons message box
  */
  /*Default*/
  lv_style_init(&style_btn_msb_box_default);
  lv_style_set_radius(&style_btn_msb_box_default, ESP3D_BUTTON_RADIUS);
  lv_style_set_bg_opa(&style_btn_msb_box_default, LV_OPA_100);
  lv_style_set_bg_color(&style_btn_msb_box_default,
                        lv_palette_main(ESP3D_BUTTON_COLOR_PALETTE));
  lv_style_set_bg_grad_color(
      &style_btn_msb_box_default,
      lv_palette_darken(ESP3D_BUTTON_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_dir(&style_btn_msb_box_default, LV_GRAD_DIR_VER);

  lv_style_set_border_opa(&style_btn_msb_box_default, LV_OPA_40);
  lv_style_set_border_width(&style_btn_msb_box_default,
                            ESP3D_BUTTON_BORDER_SIZE);
  lv_style_set_border_color(&style_btn_msb_box_default,
                            ESP3D_BUTTON_BORDER_COLOR);

  lv_style_set_outline_opa(&style_btn_msb_box_default, LV_OPA_COVER);
  lv_style_set_outline_color(
      &style_btn_msb_box_default,
      lv_palette_main(ESP3D_BUTTON_OUTLINE_COLOR_PALETTE));

  lv_style_set_text_color(&style_btn_msb_box_default, ESP3D_BUTTON_TEXT_COLOR);
  lv_style_set_pad_all(&style_btn_msb_box_default, ESP3D_BUTTON_PAD);

  /*Pressed*/

  lv_style_init(&style_btn_msb_box_pressed);

  /*Add a large outline when pressed*/
  lv_style_set_outline_width(&style_btn_msb_box_pressed,
                             ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_style_set_outline_opa(&style_btn_msb_box_pressed, LV_OPA_TRANSP);

  lv_style_set_shadow_ofs_y(&style_btn_msb_box_pressed,
                            ESP3D_BUTTON_PRESSED_SHADOW_OFFSET);
  lv_style_set_bg_color(&style_btn_msb_box_pressed,
                        lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                                          ESP3D_BUTTON_COLOR_PALETTE_DARKEN));
  lv_style_set_bg_grad_color(
      &style_btn_msb_box_pressed,
      lv_palette_darken(ESP3D_BUTTON_PRESSED_COLOR_PALETTE,
                        ESP3D_BUTTON_COLOR_PALETTE_DARKEN * 2));

  lv_style_set_text_color(&style_btn_msb_box_pressed,
                          ESP3D_BUTTON_PRESSED_TEXT_COLOR);
  lv_style_set_border_color(&style_btn_msb_box_pressed,
                            ESP3D_BUTTON_PRESSED_BORDER_COLOR);
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    /*Add a transition to the outline*/
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH,
                                      LV_STYLE_OUTLINE_OPA, LV_STYLE_PROP_INV};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear,
                                 ESP3D_BUTTON_ANIMATION_DELAY, 0, NULL);
    lv_style_set_transition(&style_btn_msb_box_pressed, &trans);
  }

  /*
  Status list
  */
  lv_style_init(&style_status_list_default);
  lv_style_set_text_opa(&style_status_list_default, LV_OPA_COVER);
  lv_style_set_text_color(&style_status_list_default,
                          ESP3D_STATUS_BAR_TEXT_COLOR);
  lv_style_set_bg_opa(&style_status_list_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_status_list_default, ESP3D_STATUS_BAR_BG_COLOR);
  lv_style_set_radius(&style_status_list_default, 0);
  // lv_style_set_pad_hor(&style_status_list_default,
  // ESP3D_STATUS_BAR_H_PAD);
  // lv_style_set_pad_ver(&style_status_list_default,
  // ESP3D_STATUS_BAR_V_PAD);
  lv_style_set_border_width(&style_status_list_default,
                            ESP3D_STATUS_BAR_BORDER_VALUE);
  lv_style_set_border_color(&style_status_list_default,
                            ESP3D_STATUS_BAR_BORDER_COLOR);
  /*
  Col Container
  */
  lv_style_init(&style_col_container_default);
  lv_style_set_radius(&style_col_container_default, ESP3D_CONTAINER_RADIUS);
  lv_style_set_layout(&style_col_container_default, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_col_container_default, LV_FLEX_FLOW_COLUMN);
  lv_style_set_flex_main_place(&style_col_container_default,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  /*
  Row Container
  */
  lv_style_init(&style_row_container_default);
  lv_style_set_radius(&style_row_container_default, ESP3D_CONTAINER_RADIUS);
  lv_style_set_layout(&style_row_container_default, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_row_container_default, LV_FLEX_FLOW_ROW);
  lv_style_set_flex_main_place(&style_row_container_default,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  /*
  Simple Container
  */
  lv_style_init(&style_simple_container_default);
  lv_style_set_bg_opa(&style_simple_container_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_simple_container_default,
                        ESP3D_SCREEN_BACKGROUND_COLOR);

  // Text Container
  lv_style_init(&style_text_container);
  lv_style_set_bg_opa(&style_text_container, LV_OPA_COVER);
  lv_style_set_bg_color(&style_text_container, ESP3D_SCREEN_BACKGROUND_COLOR);

  return true;
}

bool apply(lv_obj_t* obj, ESP3DStyleType type) {
  if (type != ESP3DStyleType::main_bg && type != ESP3DStyleType::status_list &&
      type != ESP3DStyleType::buttons_msgbox &&
      type != ESP3DStyleType::message_box &&
      type != ESP3DStyleType::radio_button) {
    lv_obj_remove_style_all(obj); /*Remove the style coming from the
    // theme*/
  }
  switch (type) {
    case ESP3DStyleType::spinner_screen:
      lv_obj_add_style(obj, &style_spinner_screen, LV_STATE_DEFAULT);
      break;
    case ESP3DStyleType::spinner_text:
      lv_obj_add_style(obj, &style_spinner_text, LV_STATE_DEFAULT);
      break;
    case ESP3DStyleType::main_bg:
      lv_obj_add_style(obj, &style_main_bg, LV_STATE_DEFAULT);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
      break;
    case ESP3DStyleType::read_only_value:
      lv_obj_add_style(obj, &style_read_only_value, LV_STATE_DEFAULT);
      break;
    case ESP3DStyleType::read_only_setting:
      lv_obj_add_style(obj, &style_read_only_setting, LV_STATE_DEFAULT);
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
    case ESP3DStyleType::progression_area:
      lv_obj_add_style(obj, &style_progression_area, LV_STATE_DEFAULT);
      lv_label_set_long_mode(obj, LV_LABEL_LONG_SCROLL_CIRCULAR);
      lv_obj_set_width(obj, ESP3D_PROGRESSION_AREA_WIDTH);
      lv_obj_set_height(obj, ESP3D_PROGRESSION_AREA_HEIGHT);
      lv_obj_set_align(obj, LV_ALIGN_CENTER);
      break;
    case ESP3DStyleType::radio_button:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
      lv_obj_add_style(obj, &style_btn_radio_default, LV_PART_INDICATOR);
      lv_obj_add_style(obj, &style_btn_radio_pressed,
                       LV_PART_INDICATOR | LV_STATE_PRESSED);
      lv_obj_add_style(obj, &style_btn_radio_checked,
                       LV_PART_INDICATOR | LV_STATE_CHECKED);
#pragma GCC diagnostic pop
      break;
    case ESP3DStyleType::button:
      lv_obj_add_style(obj, &style_btn_default, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);
      break;
    case ESP3DStyleType::buttons_msgbox:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
      lv_obj_add_style(obj, &style_btn_msb_box_default, LV_PART_ITEMS);
      lv_obj_add_style(obj, &style_btn_msb_box_pressed,
                       LV_PART_ITEMS | LV_STATE_PRESSED);
#pragma GCC diagnostic pop
      break;
    case ESP3DStyleType::buttons_matrix:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"

      lv_obj_add_style(obj, &style_btn_matrix_bar, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_btn_matrix_default, LV_PART_ITEMS);
      lv_obj_add_style(obj, &style_btn_matrix_pressed,
                       LV_PART_ITEMS | LV_STATE_PRESSED);
      lv_obj_add_style(obj, &style_btn_matrix_checked,
                       LV_PART_ITEMS | LV_STATE_CHECKED);
      lv_btnmatrix_set_btn_ctrl_all(obj, LV_BTNMATRIX_CTRL_CHECKABLE);
      lv_btnmatrix_set_one_checked(obj, true);

#pragma GCC diagnostic pop
      break;
    case ESP3DStyleType::message_box:
      lv_obj_set_width(obj, LV_PCT(80));
      ESP3DStyle::apply(lv_msgbox_get_close_btn(obj), ESP3DStyleType::button);
      ESP3DStyle::apply(lv_msgbox_get_btns(obj),
                        ESP3DStyleType::buttons_msgbox);
      lv_obj_set_height(lv_msgbox_get_btns(obj), ESP3D_SYMBOL_BUTTON_HEIGHT);
      lv_obj_set_width(lv_msgbox_get_btns(obj), ESP3D_MSGBOX_BUTTON_WIDTH);
      break;
    case ESP3DStyleType::embedded_button:
      lv_obj_add_style(obj, &style_embedded_btn_default, LV_STATE_DEFAULT);
      lv_obj_add_style(obj, &style_embedded_btn_pressed, LV_STATE_PRESSED);
      break;

    case ESP3DStyleType::row_container:
      lv_obj_add_style(obj, &style_row_container_default, LV_STATE_DEFAULT);
      lv_obj_set_style_clip_corner(obj, true, 0);
      lv_obj_set_style_pad_column(obj, ESP3D_BUTTON_PRESSED_OUTLINE,
                                  LV_PART_MAIN);
      break;

    case ESP3DStyleType::col_container:
      lv_obj_add_style(obj, &style_col_container_default, LV_STATE_DEFAULT);
      lv_obj_set_style_clip_corner(obj, true, 0);
      lv_obj_set_style_pad_top(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
      lv_obj_set_style_pad_bottom(obj, ESP3D_BUTTON_PRESSED_OUTLINE,
                                  LV_PART_MAIN);
      break;
    case ESP3DStyleType::list_container:
      ESP3DStyle::apply(obj, ESP3DStyleType::col_container);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
      lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
      lv_obj_set_style_pad_row(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
      lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN);
      lv_obj_set_style_pad_all(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
      lv_obj_add_style(obj, &style_scrollbar_default, LV_PART_SCROLLBAR);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
      lv_obj_add_style(obj, &style_scrollbar_active,
                       LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
#pragma GCC diagnostic pop
      lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
      break;
    case ESP3DStyleType::text_container:
      lv_obj_add_style(obj, &style_text_container, LV_STATE_DEFAULT);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
      break;
    case ESP3DStyleType::simple_container:
      lv_obj_add_style(obj, &style_simple_container_default, LV_STATE_DEFAULT);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
      ESP3DStyle::add_pad(obj);
      break;
    case ESP3DStyleType::status_list:
      lv_obj_add_style(obj, &style_status_list_default, LV_STATE_DEFAULT);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ELASTIC);
      break;

    default:
      return false;
  }
  return true;
}

bool add_pad(lv_obj_t* obj) {
  lv_obj_set_style_pad_left(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
  lv_obj_set_style_pad_right(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
  lv_obj_set_style_pad_top(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(obj, ESP3D_BUTTON_PRESSED_OUTLINE, LV_PART_MAIN);
  return true;
}
}  // namespace ESP3DStyle