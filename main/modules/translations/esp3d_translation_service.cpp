/*
  esp3d_translation_service
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

#include "esp3d_translation_service.h"

#include <cstdlib>
#include <map>

#include "config_file/esp3d_config_file.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_flash.h"

#define LANGUAGE_PACK "/lang_XX.lng"
#define DEFAULT_LANGUAGE "en"
#define DEFAULT_LANGUAGE_PACK "/lang_en.lng"
#define LANGUAGE_SECION "language"

#define CHUNK_BUFFER_SIZE 1024

ESP3DTranslationService esp3dTranslationService;

ESP3DTranslationService::ESP3DTranslationService() {}

ESP3DTranslationService::~ESP3DTranslationService() {}

bool ESP3DTranslationService::begin() {
  esp3d_log("Starting Translation Service");
  _translations = {{ESP3DLabel::version, "Version"},
                   {ESP3DLabel::update, "Update"}};
  // std::map<ESP3DLabel, std::string>::iterator it;
  /*
  auto it = _translations.find(ESP3DLabel::version);
  if (it != _translations.end()) {
    std::string version = it->second;
  }*/
  /*
   for(auto x: _translations)
   {
      cout << x.first << "->" <<
              x.second <<endl;
   }
  */
  std::string filename = ESP3D_FLASH_FS_HEADER;
  filename += DEFAULT_LANGUAGE_PACK;
  // TODO read setting language value
  if (flashFs.accessFS()) {
    if (flashFs.exists(filename.c_str())) {
      return true;
    }
  }
  return false;
}

bool ESP3DTranslationService::update(const char *language) { return begin(); }
void ESP3DTranslationService::handle() {}
char *ESP3DTranslationService::translate(const char *text) { return nullptr; }

/*bool ESP3DTranslationService::updateConfig() {
  bool res = false;
  ESP3DConfigFile updateConfiguration(CONFIG_FILE,
                                      esp3dUpdateService.processingFileFunction,
                                      CONFIG_FILE_OK, protectedkeys);
  if (updateConfiguration.processFile()) {
    esp3d_log("Processing ini file done");
    if (updateConfiguration.revokeFile()) {
      esp3d_log("Revoking ini file done");
      res = true;
    } else {
      esp3d_log_e("Revoking ini file failed");
    }
  } else {
    esp3d_log_e("Processing ini file failed");
  }
  return res;
}*/

void ESP3DTranslationService::end() { esp3d_log("Stop Translation Service"); }

bool ESP3DTranslationService::processingFileFunction(const char *section,
                                                     const char *key,
                                                     const char *value) {
  esp3d_log("Processing Section: %s, Key: %s, Value: %s", section, key, value);
  if (strcasecmp(section, LANGUAGE_SECION) == 0) {
    // TODO Update reference language array
    return true;
  }
  return false;
}
