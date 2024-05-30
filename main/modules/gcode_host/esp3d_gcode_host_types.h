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

/**
 * @brief Enumeration representing the state of the Gcode stream.
 */
enum class ESP3DGcodeStreamState : uint8_t {
  undefined = 0,        /**< Undefined state */
  start,                /**< Start of stream or resume */
  ready_to_read_cursor, /**< Ready to read cursor position */
  read_cursor,          /**< Read command at cursor position */
  send_gcode_command,   /**< Gcode command to send */
  resend_gcode_command, /**< Gcode command to resend */
  send_esp_command,     /**< ESP command to send */
  wait_for_ack,         /**< Wait for acknowledgement from printer */
  pause,                /**< Pause action */
  resume,               /**< Resume action */
  abort,                /**< Abort action */
  end,                  /**< End of stream reached */
  paused,               /**< Do nothing until resumed */
  error,                /**< Error state */
};

/**
 * @brief Enumeration representing the state of the Gcode host.
 */
enum class ESP3DGcodeHostState : uint8_t {
  idle = 0,   /**< Idle state */
  processing, /**< Processing state */
  paused,     /**< Paused state */
  error,      /**< Error state */
};

/**
 * @brief Enumeration representing the error codes of the Gcode host.
 */
enum class ESP3DGcodeHostError : uint8_t {
  no_error = 0,             /**< No error */
  time_out = 1,             /**< Timeout error */
  data_send = 2,            /**< Data send error */
  line_number = 3,          /**< Line number error */
  ack_number = 4,           /**< Acknowledgement number error */
  memory_allocation = 5,    /**< Memory allocation error */
  too_many_resend = 6,      /**< Too many resend error */
  number_mismatch = 7,      /**< Number mismatch error */
  line_ignored = 8,         /**< Line ignored error */
  check_sum = 9,            /**< Checksum error */
  unknow = 10,              /**< Unknown error */
  file_not_found = 11,      /**< File not found error */
  file_system = 12,         /**< File system error */
  empty_file = 13,          /**< Empty file error */
  access_denied = 14,       /**< Access denied error */
  cursor_out_of_range = 15, /**< Cursor out of range error */
  list_full = 16,           /**< List full error */
  aborted = 17,             /**< Aborted error */
  command_too_long = 18,    /**< Command too long error */
};

/**
 * @brief Enumeration representing the stream type of the Gcode host.
 */
enum class ESP3DGcodeHostStreamType : uint8_t {
  unknown,           /**< Unknown stream type */
  single_command,    /**< Single command stream type */
  multiple_commands, /**< Multiple commands stream type */
  fs_stream,         /**< File system stream type */
  fs_script,         /**< File system script type */
  sd_stream,         /**< SD card stream type */
  sd_script,         /**< SD card script type */
  invalid,           /**< Invalid stream type */
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