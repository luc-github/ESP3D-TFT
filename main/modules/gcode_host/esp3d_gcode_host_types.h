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
  ready_to_read_cursor,  // ready to read cursor position
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
  time_out = 1,
  data_send = 2,
  line_number = 3,
  ack_number = 4,
  memory_allocation = 5,
  too_many_resend = 6,
  number_mismatch = 7,
  line_ignored = 8,
  check_sum = 9,
  unknow = 10,
  file_not_found = 11,
  file_system = 12,
  empty_file = 13,
  access_denied = 14,
  cursor_out_of_range = 15,
  list_full = 16,
  aborted = 17,
  command_too_long = 18,
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

#if ESP3D_TFT_LOG
// be sure size is the same as max length of ESP3DGcodeHostStreamTypeStr
// Be sure the order is the same as ESP3DGcodeHostStreamType
const char ESP3DGcodeHostStreamTypeStr[][20] = {
    "unknown",   "single_command", "multiple_commands", "fs_stream",
    "fs_script", "sd_stream",      "sd_script",         "invalid"};

// be sure size is the same as max length of ESP3DGcodeHostStateStr
// Be sure the order is the same as ESP3DGcodeStreamState
const char ESP3DGcodeStreamStateStr[][30] = {"undefined",
                                             "start",
                                             "ready_to_read_cursor",
                                             "read_cursor",
                                             "send_gcode_command",
                                             "resend_gcode_command",
                                             "send_esp_command",
                                             "wait_for_ack",
                                             "pause",
                                             "resume",
                                             "abort",
                                             "end",
                                             "paused",
                                             "error"};
#endif  // ESP3D_TFT_LOG

#ifdef __cplusplus
}  // extern "C"
#endif