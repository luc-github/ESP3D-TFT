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
#include <ranges>

#include "config_file/esp3d_config_file.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_flash.h"

#define LANGUAGE_PACK_HEAD "ui_"
#define LANGUAGE_PACK_TAIL ".lng"
#define DEFAULT_LANGUAGE "default"
#define DEFAULT_LANGUAGE_LABEL "English (default)"
#define DEFAULT_LANGUAGE_PACK \
  LANGUAGE_PACK_HEAD DEFAULT_LANGUAGE LANGUAGE_PACK_TAIL
#define LANGUAGE_SECION "translations"
#define INFO_SECION "info"

#define CHUNK_BUFFER_SIZE 1024

ESP3DTranslationService esp3dTranslationService;

ESP3DTranslationService::ESP3DTranslationService() { _started = false; }

ESP3DTranslationService::~ESP3DTranslationService() { _started = false; }

const char *ESP3DTranslationService::getEntry(ESP3DLabel label) {
  if (!_started) {
    esp3d_log_e("Translation Service not started");
    return "???";
  }
  static std::string response;
  response = "l_" + std::to_string(static_cast<uint16_t>(label));
  return response.c_str();
}

bool ESP3DTranslationService::begin() {
  _started = false;
  esp3d_log("Starting Translation Service");
  _translations.clear();
  init();
  std::string filename = DEFAULT_LANGUAGE_PACK;
  _languageCode = DEFAULT_LANGUAGE;
  const ESP3DSettingDescription *settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_ui_language);
  if (settingPtr) {
    char out_str[(settingPtr->size) + 1] = {0};
    std::string language = esp3dTftsettings.readString(
        ESP3DSettingIndex::esp3d_ui_language, out_str, settingPtr->size);
    filename = LANGUAGE_PACK_HEAD;
    filename += language;
    filename += LANGUAGE_PACK_TAIL;
    _languageCode = language;
  }
  std::string language;
  if (filename == DEFAULT_LANGUAGE_PACK) {
    esp3d_log("Using default language pack: %s",
              translate(ESP3DLabel::language));
    _started = true;
    esp3d_log("Start is %s", _started ? "Ok" : "KO");
    return _started;
  }
  esp3d_log("Using language pack: %s over %s", filename.c_str(),
            DEFAULT_LANGUAGE_PACK);
  if (flashFs.accessFS()) {
    if (flashFs.exists(filename.c_str())) {
      filename = ESP3D_FLASH_FS_HEADER + filename;
      ESP3DConfigFile updateTranslations(
          filename.c_str(), esp3dTranslationService.processingFileFunction);
      esp3d_log("Processing language pack: %s", filename.c_str());
      if (updateTranslations.processFile()) {
        esp3d_log("Processing language pack, done: %s",
                  translate(ESP3DLabel::language));
        flashFs.releaseFS();
        _started = true;
      } else {
        esp3d_log_e("Processing language pack, failed");
      }

    } else {
      esp3d_log_e("Cannot find the language file: %s", filename.c_str());
    }
    flashFs.releaseFS();
  } else {
    esp3d_log_e("Cannot access local fs");
  }
  if (!_started) {
    esp3d_log_e("Translation Service not started");
    _translations.clear();
    init();
    _languageCode = DEFAULT_LANGUAGE;
    _started = true;
  }
  esp3d_log("Start is %s", _started ? "Ok" : "KO");
  return _started;
}

std::vector<std::string> ESP3DTranslationService::getLanguagesLabels() {
  return _labels;
}

std::vector<std::string> ESP3DTranslationService::getLanguagesValues() {
  return _values;
}

uint16_t ESP3DTranslationService::getLanguagesList() {
  if (!_started) {
    return 0;
  }
  uint16_t count = 1;
  _values.clear();
  _labels.clear();
  _values.push_back(DEFAULT_LANGUAGE);
  _labels.push_back(DEFAULT_LANGUAGE_LABEL);

  if (flashFs.accessFS()) {
    DIR *dir = flashFs.opendir("/");
    if (dir) {
      struct dirent *entry;
      while ((entry = flashFs.readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
          continue;
        } else {
          if (esp3d_string::startsWith(entry->d_name, LANGUAGE_PACK_HEAD) &&
              esp3d_string::endsWith(entry->d_name, LANGUAGE_PACK_TAIL)) {
            esp3d_log("Found file: %s", entry->d_name);
            std::string languageCode = entry->d_name;
            languageCode.erase(0, strlen(LANGUAGE_PACK_HEAD));
            languageCode.erase(
                languageCode.length() - strlen(LANGUAGE_PACK_TAIL),
                strlen(LANGUAGE_PACK_TAIL));
            std::string filename = ESP3D_FLASH_FS_HEADER;
            filename += entry->d_name;
            ESP3DConfigFile getLanguage(filename.c_str());
            char tmpstr[255] = {0};
            if (getLanguage.processFile(LANGUAGE_SECION,
                                        getEntry(ESP3DLabel::language), tmpstr,
                                        sizeof(tmpstr))) {
              if (strlen(tmpstr) > 0) {
                esp3d_log("Found file: %s, code: %s, language:%s",
                          entry->d_name, languageCode.c_str(), tmpstr);
                _values.push_back(languageCode);
                _labels.push_back(tmpstr);
                count++;
                esp3d_log("entry: %d, Found file: %s, code: %s, language:%s",
                          count, entry->d_name, _values[count - 1].c_str(),
                          _labels[count - 1].c_str());
              }
            }
          }
        }
        esp3d_hal::wait(0);
      }
      flashFs.closedir(dir);
    }
    flashFs.releaseFS();
  }
  return count;
}

void ESP3DTranslationService::handle() {}

const char *ESP3DTranslationService::translate(ESP3DLabel label, ...) {
  static std::string responseString;
  responseString.clear();
  if (!_started) {
    esp3d_log_e("Translation Service not started");
    return "???";
  }
  auto it = _translations.find(label);
  if (it != _translations.end()) {
    esp3d_log("Key index: %d is found in translation and text is %s",
              static_cast<uint16_t>(label), it->second.c_str());
    char localBuffer[64] = {0};
    char *buffer = localBuffer;
    va_list args;
    va_list copy;
    // Disable warning for va_start
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvarargs"
    va_start(args, (char *)(it->second.c_str()));
#pragma GCC diagnostic pop
    va_copy(copy, args);
    size_t len = vsnprintf(NULL, 0, it->second.c_str(), args);
    va_end(copy);
    if (len >= sizeof(localBuffer)) {
      buffer = (char *)malloc(sizeof(char) * (len + 1));
      if (buffer == nullptr) {
        esp3d_log_e("Cannot allocate memory for translation");
        return "???";
      }
    }
    len = vsnprintf(buffer, len + 1, it->second.c_str(), args);
    responseString = buffer;
    va_end(args);
    if (buffer != localBuffer) {
      free(buffer);
    }
    return responseString.c_str();
  }
  esp3d_log_e("Key index: %d is not found in translation",
              static_cast<uint16_t>(label));
  return "???";
}

void ESP3DTranslationService::end() { esp3d_log("Stop Translation Service"); }

bool ESP3DTranslationService::updateTranslation(const char *text,
                                                ESP3DLabel label) {
  auto it = _translations.find(static_cast<ESP3DLabel>(label));
  if (it != _translations.end()) {
    esp3d_log("Key index: %d is found in translation and updated to %s",
              static_cast<uint16_t>(label), text);
    it->second = text;

    return true;
  } else {
    esp3d_log_e("Key index: %d is not found in translation",
                static_cast<uint16_t>(label));
    return false;
  }
}

bool ESP3DTranslationService::processingFileFunction(const char *section,
                                                     const char *key,
                                                     const char *value) {
  if (strcasecmp(section, LANGUAGE_SECION) == 0) {
    // TODO Update reference language array
    esp3d_log("Processing Section: %s, Key: %s, Value: %s", section, key,
              value);
    if (strlen(value) >= 1) {
      uint16_t keyIndex = atoi(&key[2]);
      if (keyIndex >= static_cast<uint16_t>(ESP3DLabel::unknown_index)) {
        esp3d_log_e("Key index: %d is invalid", keyIndex);
        return false;
      }
      esp3d_log("Key index: %d is valid ", keyIndex);
      return esp3dTranslationService.updateTranslation(
          value, static_cast<ESP3DLabel>(keyIndex));
    } else {
      esp3d_log_e("Key index: %s is invalid", key);
      return false;
    }
  } else {
    if (strcasecmp(section, INFO_SECION) == 0) {
      esp3d_log_e("Section: %s is ignored", section);
      return true;
    }
    esp3d_log_e("Section: %s is invalid", section);
    return true;
  }
  return false;
}