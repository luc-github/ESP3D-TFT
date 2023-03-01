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

typedef enum {
    ESP3D_NO_WAIT = 0,
    ESP3D_WAIT_ACK,
    ESP3D_WAIT_BUSY,
    ESP3D_WAIT_PROCESSING,
    ESP3D_WAIT_HEATING
} esp3d_gcode_host_wait_t;

typedef enum {
    ESP3D_HOST_NO_STREAM = 0,
    ESP3D_HOST_START,
    ESP3D_HOST_END,
    ESP3D_HOST_READ_LINE,
    ESP3D_HOST_PROCESS_LINE,
    ESP3D_HOST_WAIT4_ACK,
    ESP3D_HOST_PAUSE_STREAM,
    ESP3D_HOST_PAUSED_STREAM,
    ESP3D_HOST_RESUME_STREAM,
    ESP3D_HOST_STOP,
    ESP3D_HOST_ERROR,
    ESP3D_HOST_ABORT,
    ESP3D_HOST_WAIT,
    ESP3D_HOST_NEXT_STATE
} esp3d_gcode_host_state_t;

typedef enum {
    ESP3D_NO_ERROR_STREAM =          0,
    ESP3D_ERROR_TIME_OUT,
    ESP3D_ERROR_CANNOT_SEND_DATA,
    ESP3D_ERROR_LINE_NUMBER,
    ESP3D_ERROR_ACK_NUMBER,
    ESP3D_ERROR_MEMORY_PROBLEM,
    ESP3D_ERROR_RESEND,
    ESP3D_ERROR_NUMBER_MISMATCH,
    ESP3D_ERROR_LINE_IGNORED,
    ESP3D_ERROR_FILE_SYSTEM,
    ESP3D_ERROR_CHECKSUM,
    ESP3D_ERROR_UNKNOW,
    ESP3D_ERROR_FILE_NOT_FOUND,
    ESP3D_ERROR_STREAM_ABORTED
} esp3d_gcode_host_error_t;

typedef enum {
    ESP3D_TYPE_SINGLE_COMMAND,
    ESP3D_TYPE_SCRIPT_STREAM,
    ESP3D_TYPE_FS_STREAM,
    ESP3D_TYPE_SD_STREAM,
} esp3d_gcode_host_script_type_t;

#ifdef __cplusplus
} // extern "C"
#endif