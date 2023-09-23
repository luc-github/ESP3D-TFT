/*
  esp3d_gcode_host

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

#ifdef __cplusplus
extern "C" {
#endif

enum class ESP3DGcodeStreamState : uint8_t {
  undefined = 0,
  start,                 // start of stream / or resume it
  read_cursor,           // read command at cursor position
  send_gcode_command,    // gcode command to send
  resend_gcode_command,  // gcode command to resend
  send_esp_command,      // esp command to send
  wait_for_ack,          // wait for ack from pr
  pause,                 // pause action
  resume,                // resume action
  abort,                 // abort action
  end,                   // end of stream reached
  paused,                // do nothing until resumed
  error,                 // error state
};

enum class ESP3DGcodeHostState : uint8_t {
  idle = 0,
  processing,
  paused,
  error,
};

enum class ESP3DGcodeHostError : uint8_t {
  no_error = 0,
  time_out,
  data_send,
  line_number,
  ack_number,
  memory_allocation,
  too_many_resend,
  number_mismatch,
  line_ignored,
  check_sum,
  unknow,
  file_not_found,
  file_system,
  empty_file,
  access_denied,
  cursor_out_of_range,
  aborted
};

enum class ESP3DGcodeHostStreamType : uint8_t {
  unknown,
  single_command,
  multiple_commands,
  fs_stream,
  fs_script,
  sd_stream,
  sd_script,
  invalid,
};

#ifdef __cplusplus
}  // extern "C"
#endif