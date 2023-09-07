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

enum class ESP3DGcodeHostWait : uint8_t {
  no_wait = 0,
  wait_ack,
  busy,
  processing,
  heating
};

enum class ESP3DGcodeHostState : uint8_t {
  no_stream = 0,
  start,
  end,
  read_line,
  process_line,
  wait_for_ack,
  pause,
  paused,
  resume,
  stop,
  error,
  abort,
  wait,
  next_state,
  undefined,
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

enum class ESP3DGcodeHostScriptType : uint8_t {
  single_command,
  multiple_commands,
  file_commands,
  filesystem,
  sd_card,
  active,
  unknown
};

#ifdef __cplusplus
}  // extern "C"
#endif