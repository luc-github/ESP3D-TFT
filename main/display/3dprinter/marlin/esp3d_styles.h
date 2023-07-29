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

#ifdef __cplusplus
extern "C" {
#endif

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
};

extern bool init_styles();
extern bool apply_style(lv_obj_t* obj, ESP3DStyleType type);
extern bool apply_outline_pad(lv_obj_t* obj);

#ifdef __cplusplus
}  // extern "C"
#endif
