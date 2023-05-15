/*
  esp3d_translation_service

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

#include <map>

#include "esp3d_settings.h"
#include "esp3d_string.h"

#ifdef __cplusplus
extern "C" {
#endif

enum class ESP3DLabel : uint16_t { version = 0, update, unknown_index };

class ESP3DTranslationService final {
 public:
  ESP3DTranslationService();
  ~ESP3DTranslationService();
  bool begin();
  bool update(const char *language);
  void handle();
  char *translate(const char *text);
  void end();
  static bool processingFileFunction(const char *section, const char *key,
                                     const char *value);

 private:
  std::map<ESP3DLabel, std::string> _translations;
};

extern ESP3DTranslationService esp3dTranslationService;
#ifdef __cplusplus
}  // extern "C"
#endif