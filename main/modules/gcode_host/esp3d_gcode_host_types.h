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
  // no_stream = 0,
  start,
  end,
  read_line,      // maybe should rename
  wait_for_send,  // wait for currentCommand to be sent, so we can read the next
                  // command ready.
  pause,  // only needed for file stream, maybe better to have another class?
  resume,
  abort,
  stop,    // stop vs abort?
  paused,  // do nothing until resumed
  error,
  undefined,
  processing,
};

enum class ESP3DGcodeHostState : uint8_t {  // maybe need stream state and host
                                            // state?
  idle = 0,                                 // wait for a command to send
  send_gcode_command,  // if waiting not waiting for ack send command
  send_esp_command,    //
  wait_for_ack,
  error,
  abort,
  undefined,
  paused,  // only used to indicate print stream is paused outside of gcodehost,
           // not used here
  processing,
};

enum class ESP3DGcodeHostError : uint8_t {
  no_error = 0,
  time_out,
  data_send,
  line_number,
  ack_number,
  memory_allocation,
  resend,
  number_mismatch,
  line_ignored,
  file_system,
  check_sum,
  unknow,
  file_not_found,
  aborted
};

enum class ESP3DGcodeHostFileType : uint8_t {
  single_command,
  multiple_commands,  // no real difference between single and multiple in
                      // practice
  filesystem,
  script,
  sd_card,
  sd_script,
  active,
  unknown,
  invalid
};

#ifdef __cplusplus
}  // extern "C"
#endif