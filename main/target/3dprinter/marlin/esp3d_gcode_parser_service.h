/*
  esp3d_gcode_parser_service

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

#include "esp3d_data_type.h"
#include "esp3d_string.h"

#ifdef __cplusplus
extern "C" {
#endif

enum class FW_GCodeCommand : uint8_t {
  reset_stream_numbering = 0,
};

#define ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_TEMPERATURE 0
#define ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_POSITION 1
#define ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_SPEED 2

#define ESP3D_POLLING_COMMANDS_COUNT 3

class ESP3DGCodeParserService final {
 public:
  ESP3DGCodeParserService();
  ~ESP3DGCodeParserService();
  ESP3DDataType getType(const char *data);
  bool hasMultiLineReport(const char *data);
  const char *getLastError() { return _lastError.c_str(); }
  uint64_t getLineResend() { return _lineResend; }
  bool processCommand(const char *data);
  const char **getPollingCommands();
  const char *getFwCommandString(FW_GCodeCommand cmd);
  bool hasAck(const char *command);
  bool forwardToScreen(const char *command);
  bool isAckNeeded() { return true; }  // Depend on FW
  uint64_t getPollingCommandsLastRun(uint8_t index);
  bool setPollingCommandsLastRun(uint8_t index, uint64_t value);

 private:
  bool _isMultiLineReportOnGoing;
  std::string _lastError;
  uint64_t _lineResend;
};

extern ESP3DGCodeParserService esp3dGcodeParser;
#ifdef __cplusplus
}  // extern "C"
#endif