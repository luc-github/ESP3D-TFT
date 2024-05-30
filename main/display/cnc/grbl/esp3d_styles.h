/*
  esp3d_styles.h - ESP3D screens styles definition

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

#pragma once

#include <stdio.h>

#include "esp3d_styles_res.h"
#include "lvgl.h"

// Styles for common controls
enum class ESP3DStyleType : uint8_t {
  default_style = 0,
  main_bg,
  button,
  embedded_button,
  row_container,
  col_container,
  simple_container,
  status_bar,
  status_list,
  bg_label,
  buttons_matrix,
  read_only_value,
  read_only_setting,
  message_box,
  buttons_msgbox,
  spinner_screen,
  spinner_text,
  radio_button,
  list_container,
  text_container,
  progression_area,
};

// Name space for styles
namespace ESP3DStyle {
bool init();
bool apply(lv_obj_t* obj, ESP3DStyleType type);
bool add_pad(lv_obj_t* obj);
};  // namespace ESP3DStyle

// Colors definition

// Screen colors
#define ESP3D_SCREEN_BACKGROUND_COLOR lv_color_hex(0x000000)
#define ESP3D_SCREEN_BACKGROUND_TEXT_COLOR lv_color_hex(0xFFFFFF)

// Button colors
#define ESP3D_BUTTON_COLOR_PALETTE LV_PALETTE_GREY
#define ESP3D_BUTTON_COLOR_PALETTE_DARKEN 2
#define ESP3D_BUTTON_BORDER_COLOR lv_color_hex(0xFFFFFF)
#define ESP3D_BUTTON_TEXT_COLOR lv_color_hex(0xFFFFFF)
#define ESP3D_BUTTON_OUTLINE_COLOR_PALETTE LV_PALETTE_GREEN
#define ESP3D_BUTTON_PRESSED_COLOR_PALETTE LV_PALETTE_GREY
#define ESP3D_BUTTON_PRESSED_TEXT_COLOR lv_color_hex(0x00FF00)
#define ESP3D_BUTTON_PRESSED_BORDER_COLOR lv_color_hex(0x00FF00)

// Status bar colors
#define ESP3D_STATUS_BAR_TEXT_COLOR lv_color_hex(0x000000)
#define ESP3D_STATUS_BAR_BG_COLOR lv_color_hex(0xFFFFFF)
#define ESP3D_STATUS_BAR_BORDER_COLOR lv_palette_main(LV_PALETTE_GREY)

// Progression area colors
#define ESP3D_PROGRESSION_AREA_TEXT_COLOR lv_color_hex(0x000000)
#define ESP3D_PROGRESSION_AREA_BG_COLOR lv_color_hex(0xFFFFFF)
#define ESP3D_PROGRESSION_AREA_BORDER_COLOR lv_palette_main(LV_PALETTE_GREY)
