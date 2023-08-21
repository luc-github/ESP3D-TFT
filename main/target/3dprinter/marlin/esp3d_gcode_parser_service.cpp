/*
  esp3d_gcode_parser_service
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

#include "esp3d_gcode_parser_service.h"

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"

ESP3DGCodeParserService esp3dGcodeParser;

ESP3DGCodeParserService::ESP3DGCodeParserService() {
  _isMultiLineReportOnGoing = false;
}

ESP3DGCodeParserService::~ESP3DGCodeParserService() {
  _isMultiLineReportOnGoing = false;
}

ESP3DDataType ESP3DGCodeParserService::getType(const char* data) {
  if (data == nullptr) {
    return ESP3DDataType::empty_line;
  }
  char* ptr = (char*)data;

  // remove leading spaces and tabs
  for (uint8_t i = 0; i < strlen(data); i++) {
    if (data[i] == ' ' || data[i] == '\t') {
      ptr++;
    } else {
      break;
    }
  }

  // is empty line ?
  if (ptr[0] == '\n' || ptr[0] == '\r') {
    return ESP3DDataType::empty_line;
  }

  // is it ack ?
  if ((ptr[0] == 'o' && ptr[1] == 'k') &&
      (ptr[2] == '\n' || ptr[2] == '\r' || ptr[2] == ' ')) {
    return ESP3DDataType::ack;
  }

  // is it comment ?
  if (ptr[0] == ';' || ptr[0] == '#') {
    return ESP3DDataType::comment;
  }

  // is gcode ?
  if (ptr[0] == 'M' || ptr[0] == 'G' || ptr[0] == 'T') {
    char* ptr2 = ptr;
    int hasNumber = 0;
    for (uint8_t i = 0; i < strlen(ptr2); i++) {
      if (ptr2[i] >= '0' && ptr2[i] <= '9') {
        hasNumber++;
      } else if ((ptr2[i] == ' ' || ptr2[i] == '\t' || ptr2[i] == '\n' ||
                  ptr2[i] == '\r') &&
                 hasNumber > 0 && hasNumber < 5) {
        return ESP3DDataType::gcode;
      } else {
        // not a gcode
        break;
      }
    }
  }

  // is it resend ?
  if (strstr(ptr, "Resend: ") == ptr) {
    return ESP3DDataType::resend;
  }

  // is it error ?
  if (strstr(ptr, "Error:") == ptr) {
    return ESP3DDataType::error;
  }

  // is it status ?
  if (strstr(ptr, "busy") == ptr || strstr(ptr, "processing") == ptr ||
      strstr(ptr, "heating") == ptr || strstr(ptr, "echo:busy") == ptr ||
      strstr(ptr, "echo:processing") == ptr ||
      strstr(ptr, "echo:heating") == ptr) {
    return ESP3DDataType::status;
  }
  /*
    "Not SD printing"
    "Done printing file"
    "SD printing byte"
    "echo: M73 Progress:"
    "echo:Print time:"
    "echo:E"
    "Current file:"
    "FR:"
    "Cap:"
    "FIRMWARE_NAME:"
    "ok T:25.00 /120.00 B:25.00 /0.00 @:127 B@:0"
    "T:25.00 /0.00 B:25.00 /50.00 T0:25.00 /0.00 T1:25.00 /0.00 @:0 B@:127 @0:0
    @1:0"
    "X:0.00 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0"
    */

  // is it response ?
  if (strstr(ptr, "echo:") == ptr || strstr(ptr, "ok T:") == ptr ||
      strstr(ptr, "T:") == ptr || strstr(ptr, "X:") == ptr ||
      strstr(ptr, "Not SD printing") == ptr ||
      strstr(ptr, "Done printing file") == ptr ||
      strstr(ptr, "SD printing byte") == ptr ||
      strstr(ptr, "Current file:") == ptr || strstr(ptr, "FR:") == ptr ||
      strstr(ptr, "Cap:") == ptr || strstr(ptr, "FIRMWARE_NAME:") == ptr) {
    return ESP3DDataType::response;
  }

  // is [ESPxxx] command ?
  if (strstr(ptr, "[ESP") == ptr) {
    if (strlen(ptr) >= 5 &&
        (ptr[4] == ']' || (ptr[4] >= '0' && ptr[4] <= '9'))) {
      return ESP3DDataType::esp_command;
    }
  }

  return ESP3DDataType::unknown;
}
bool ESP3DGCodeParserService::needAck(const char* data) { return false; }
