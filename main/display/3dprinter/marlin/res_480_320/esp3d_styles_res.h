/*
  esp3d_styles_res.h - ESP3D screens styles definition

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

#define CURRENT_STATUS_BAR_RADIUS 5
#define CURRENT_STATUS_BAR_H_PAD 5
#define CURRENT_STATUS_BAR_V_PAD 3
#define CURRENT_STATUS_BAR_BORDER_VALUE 1
#define CURRENT_STATUS_AREA_HEIGHT 50
#define CURRENT_STATUS_AREA_WIDTH LV_HOR_RES - (2 * 15)
#define CURRENT_CHOICE_LIST_WIDTH LV_HOR_RES / 3

#define CURRENT_BUTTON_RADIUS_VALUE 5
#define CURRENT_BUTTON_BORDER_VALUE 2

#define CURRENT_BUTTON_PRESSED_OUTLINE 10
#define CURRENT_BUTTON_COLOR_PRESSED_SHADOW_OFFSET 3
#define CURRENT_BUTTON_PAD 4

#define CURRENT_CONTAINER_RADIUS 5

#define BUTTON_WIDTH (LV_HOR_RES / 6)
#define BUTTON_HEIGHT BUTTON_WIDTH

#define SYMBOL_BUTTON_WIDTH 70
#define SYMBOL_BUTTON_HEIGHT 56

#ifndef BUTTON_ANIMATION_DELAY
#define BUTTON_ANIMATION_DELAY 300
#endif  // BUTTON_ANIMATION_DELAY

#define CURRENT_SPINNER_SIZE 100

#define BACK_BUTTON_WIDTH 70
#define BACK_BUTTON_HEIGHT 50
#define MATRIX_BUTTON_WIDTH 70
#define MATRIX_BUTTON_HEIGHT 50
#define MSGBOX_BUTTON_WIDTH 130

#define LIST_CONTAINER_LR_PAD 10
#define LIST_LINE_HEIGHT 45
#define LIST_LINE_BORDER_WIDTH 3
#define LIST_LINE_BUTTON_WIDTH 50

#define CURRENT_SCROLL_BAR_WIDTH 10
#define CURRENT_SCROLL_BAR_RADIUS 5
