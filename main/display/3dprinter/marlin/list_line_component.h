/*
 list_line_component.h - ESP3D screens definition

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

#include <string>

#include "esp3d_styles.h"
#include "esp3d_values.h"

#ifdef __cplusplus
extern "C" {
#endif
namespace listLine {
extern lv_obj_t *create_list_line_container(lv_obj_t *container);
lv_obj_t *add_label_to_line(const char *lbl, lv_obj_t *line_container,
                            bool grow);
lv_obj_t *add_button_to_line(const char *lbl, lv_obj_t *line_container);
}  // namespace listLine

#ifdef __cplusplus
}  // extern "C"
#endif
