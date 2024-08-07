
/*
  esp3d_values.h -  values esp3d functions class

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

#pragma once
#include <stdio.h>

#include <functional>
#include <list>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#if !ESP3D_DISPLAY_FEATURE
#define LV_SYMBOL_STATION_MODE "S"
#define LV_SYMBOL_ACCESS_POINT "A"
#define LV_SYMBOL_BLUETOOTH "B"
#define LV_SYMBOL_WIFI "O"
#endif  // ESP3D_DISPLAY_FEATURE

// this list depend of target feature
enum class ESP3DValuesIndex : uint16_t {
  status_bar_label,
  current_ip,
  m_position_x,
  m_position_y,
  m_position_z,
  m_position_a,
  m_position_b,
  m_position_c,
  w_position_x,
  w_position_y,
  w_position_z,
  w_position_a,
  w_position_b,
  w_position_c,
  job_status,
  file_path,
  file_name,
  network_status,
  network_mode,
  job_progress,
  job_duration,
  job_id,
  state,
  state_comment,
  unknown_index
};

#ifdef __cplusplus
}  // extern "C"
#endif