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
//Styles for common controls

//Back button size
#define ESP3D_BACK_BUTTON_WIDTH 50
#define ESP3D_BACK_BUTTON_HEIGHT 40

//Buttons
#define ESP3D_BUTTON_RADIUS  4
#define ESP3D_BUTTON_BORDER_SIZE 2
#define ESP3D_BUTTON_PRESSED_OUTLINE 5
#define ESP3D_BUTTON_PRESSED_SHADOW_OFFSET 3
#define ESP3D_BUTTON_PAD 3
#define ESP3D_BUTTON_WIDTH 60
#define ESP3D_BUTTON_HEIGHT 60
#ifndef ESP3D_BUTTON_ANIMATION_DELAY
#define ESP3D_BUTTON_ANIMATION_DELAY 300
#endif  // ESP3D_BUTTON_ANIMATION_DELAY

//Symbol buttons
#define ESP3D_SYMBOL_BUTTON_WIDTH 50
#define ESP3D_SYMBOL_BUTTON_HEIGHT 44

//Matrix buttons
#define ESP3D_MATRIX_BUTTON_WIDTH 48
#define ESP3D_MATRIX_BUTTON_HEIGHT 40

//Message box buttons
#define ESP3D_MSGBOX_BUTTON_WIDTH 130

//Spinner size
#define ESP3D_SPINNER_SIZE 70

//List styles
#define ESP3D_LIST_CONTAINER_LR_PAD 6
#define ESP3D_LIST_LINE_HEIGHT 36
#define ESP3D_LIST_LINE_BORDER_SIZE 2
#define ESP3D_LIST_LINE_BUTTON_WIDTH 40

//Container styles
#define ESP3D_CONTAINER_RADIUS 5

//Scroll bar styles
#define ESP3D_SCROLL_BAR_WIDTH 10
#define ESP3D_SCROLL_BAR_RADIUS 5

//Choice List styles
#define ESP3D_CHOICE_LIST_WIDTH LV_HOR_RES / 2

// Status bar styles
#define ESP3D_STATUS_BAR_RADIUS 4
#define ESP3D_STATUS_BAR_H_PAD 5
#define ESP3D_STATUS_BAR_V_PAD 3
#define ESP3D_STATUS_BAR_BORDER_VALUE 1
#define ESP3D_STATUS_BAR_HEIGHT 50
#define ESP3D_STATUS_BAR_WIDTH LV_HOR_RES - (2 * 10)

//Progression area styles
#define ESP3D_PROGRESSION_AREA_RADIUS 4
#define ESP3D_PROGRESSION_AREA_H_PAD 5
#define ESP3D_PROGRESSION_AREA_V_PAD 3
#define ESP3D_PROGRESSION_AREA_BORDER_VALUE 1
#define ESP3D_PROGRESSION_AREA_HEIGHT 50
#define ESP3D_PROGRESSION_AREA_WIDTH (LV_HOR_RES - (2 * 10))


