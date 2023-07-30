/*
  esp3d_translation_service

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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

#ifdef __cplusplus
extern "C" {
#endif

enum class ESP3DLabel : uint16_t {
  language = 0,
  size_for_update,
  screen,
  architecture,
  sdk_version,
  cpu_freq,
  flash_size,
  free_heap,
  total_psram,
  version,
  sd_updater,
  on,
  off,
  millimeters,
  celcius,
  flash_type,
  confirm,
  stop_print,
  unknown_index
};

#ifdef __cplusplus
}  // extern "C"
#endif