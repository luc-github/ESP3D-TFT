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
#include <vector>

#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_translations_list.h"

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DTranslationService final {
 public:
  ESP3DTranslationService();
  ~ESP3DTranslationService();
  bool begin();
  void init();
  void handle();
  const char *translate(ESP3DLabel label, ...);
  uint16_t getLanguagesList();
  std::vector<std::string> getLanguagesLabels();
  std::vector<std::string> getLanguagesValues();
  void end();
  static bool processingFileFunction(const char *section, const char *key,
                                     const char *value);
  bool updateTranslation(const char *text, ESP3DLabel label);
  const char *getEntry(ESP3DLabel label);
  const char *getLanguageCode() { return _languageCode.c_str(); }
  bool started() { return _started; }

 private:
  bool _started;
  std::map<ESP3DLabel, std::string> _translations;
  std::string _languageCode;
  std::vector<std::string> _values;
  std::vector<std::string> _labels;
};

extern ESP3DTranslationService esp3dTranslationService;
#ifdef __cplusplus
}  // extern "C"
#endif