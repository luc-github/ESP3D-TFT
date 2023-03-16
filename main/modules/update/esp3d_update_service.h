/*
  esp3d_update_service

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

#include "esp3d_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

class ESP3dUpdateService final {
 public:
  ESP3dUpdateService();
  ~ESP3dUpdateService();
  bool begin();
  void handle();
  void end();
#if ESP3D_SD_CARD_FEATURE
  bool updateFW();
#endif  // ESP3D_SD_CARD_FEATURE
  bool updateConfig();
  bool canUpdate();
  size_t maxUpdateSize();
  static bool processingFileFunction(const char* section, const char* key,
                                     const char* value);

 private:
  bool _canUpdate;
  size_t _maxUpdateSize;
  static bool processString(const char** keysval,
                            const Esp3dSettingIndex* keypos, const size_t size,
                            const char* key, const char* value, char& T,
                            Esp3dSettingIndex& P);
  static bool processInt(const char** keysval, const Esp3dSettingIndex* keypos,
                         const size_t size, const char* key, const char* value,
                         char& T, Esp3dSettingIndex& P, uint32_t& v);
  static bool processBool(const char** keysval, const Esp3dSettingIndex* keypos,
                          const size_t size, const char* key, const char* value,
                          char& T, Esp3dSettingIndex& P, uint8_t& b);
};

extern ESP3dUpdateService esp3dUpdateService;
#ifdef __cplusplus
}  // extern "C"
#endif