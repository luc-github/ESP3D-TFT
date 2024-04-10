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
// Do not change order and avoid comment
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
  celsius,
  flash_type,
  confirmation,
  stop_print,
  error,
  error_applying_mode,
  error_applying_setting,
  hostname,
  extensions,
  please_wait,
  output_client,
  usb,
  serial,
  serial_baud_rate,
  usb_baud_rate,
  connecting,
  not_connected,
  ap_mode,
  ip_lost,
  jog_type,
  absolute_short,
  absolute,
  relative_short,
  relative,
  polling,
  enabled,
  disabled,
  motors_disabled,
  fan_controls,
  information,
  auto_leveling,
  manual_leveling_help,
  manual_leveling_text,
  bed_width,
  bed_depth,
  auto_bed_probing,
  start_probing,
  invert_axis,
  ui_language,
  no_sd_card,
  communication_lost,
  communication_recovered,
  ms,
  days,
  streaming_error,
  command_error,
  stream_error,
  target_firmware,
  unknown_index,  // must be last
};

#ifdef __cplusplus
}  // extern "C"
#endif