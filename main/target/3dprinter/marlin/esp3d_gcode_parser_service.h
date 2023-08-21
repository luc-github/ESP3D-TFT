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

class ESP3DGCodeParserService final {
 public:
  ESP3DGCodeParserService();
  ~ESP3DGCodeParserService();
  ESP3DDataType getType(const char* data);
  bool needAck(const char* data);

 private:
  bool _isMultiLineReportOnGoing;
};

extern ESP3DGCodeParserService esp3dGcodeParser;
#ifdef __cplusplus
}  // extern "C"
#endif