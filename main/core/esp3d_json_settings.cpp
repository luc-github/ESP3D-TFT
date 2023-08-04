
/*
  esp3d_json_settings.cpp -  settings esp3d functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "esp3d_json_settings.h"

#include <cstring>
#include <regex>
#include <string>

#include "esp3d_log.h"
#include "filesystem/esp3d_flash.h"
#define PREFERENCES_JSON_FILE "preferences.json"
#define CHUNK_BUFFER_SIZE 255
ESP3DJsonSettings esp3dTftJsonSettings;

ESP3DJsonSettings::ESP3DJsonSettings() {}
ESP3DJsonSettings::~ESP3DJsonSettings() {}

const char* ESP3DJsonSettings::readString(const char* section,
                                          const char* entry, char* out_str,
                                          size_t len, bool* haserror) {
  static std::string str;
  FILE* prefsHandle = flashFs.open(PREFERENCES_JSON_FILE, "rb");
  if (prefsHandle == NULL) {
    esp3d_log_e("Failed to open " PREFERENCES_JSON_FILE);
    if (haserror != NULL) *haserror = true;
    return "";
  }
  size_t chunksize;

  char chunk[CHUNK_BUFFER_SIZE + 1];
  std::string currentLabel = "";
  bool initialString = true;
  bool waitForBoolean = false;
  uint deepthLevel = 0;
  int currentIndex = 0;
  int valueIndex = 0;
  bool valueIsString = false;
  std::string currentSection = "";
  std::string currentParsingStr = "";
  std::string valueSearched = "";
  currentIndex = 0;
  bool parsingDone = false;
  do {
    chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, prefsHandle);

    if (chunksize > 0) {
      chunk[chunksize] = 0;
      for (size_t i = 0; i < chunksize; i++) {
        if (parsingDone) break;
        switch (chunk[i]) {
          case '[':
          case '{':
            waitForBoolean = false;

            deepthLevel++;
            // is section label?
            if (currentLabel.length() > 0) {
              esp3d_log("Entering in section '%s'", currentLabel.c_str());
            }
            currentLabel = "";
            currentParsingStr = "";
            break;
          case ']':
          case '}':
            deepthLevel--;
            if (waitForBoolean) {
              std::string key = currentLabel;
              std::string value = currentParsingStr;
              if (key == entry) {
                valueIsString = false;
                parsingDone = true;
                valueSearched = value;
                valueIndex = currentIndex - value.length() - 1;
                esp3d_log("Found");
              }
              esp3d_log("Setting %s = %s", key.c_str(), value.c_str());
              currentLabel = "";
              currentParsingStr = "";
            }
            currentLabel = "";
            currentParsingStr = "";
            waitForBoolean = false;
            break;
          case '\'':
          case '"':
            waitForBoolean = false;
            if (initialString) {
              initialString = false;
              currentParsingStr = "";
            } else {
              initialString = true;

              if (currentLabel.length() == 0) {
                currentLabel = currentParsingStr;
              } else {
                std::string key = currentLabel;
                std::string value = currentParsingStr;
                if (key == entry) {
                  valueIsString = true;
                  parsingDone = true;
                  valueIndex = currentIndex - value.length() - 1;
                  valueSearched = value;
                  esp3d_log("Found");
                }
                esp3d_log("Setting %s = %s", key.c_str(), value.c_str());
                currentLabel = "";
              }
              currentParsingStr = "";
            }

            break;
          case ':':
            waitForBoolean = true;
            break;
          case ',':
            if (waitForBoolean) {
              std::string key = currentLabel;
              std::string value = currentParsingStr;
              if (key == entry) {
                valueIsString = false;
                parsingDone = true;
                valueIndex = currentIndex - value.length() - 1;
                valueSearched = value;
                esp3d_log("Found");
              }
              esp3d_log("Setting %s = %s", key.c_str(), value.c_str());
              currentLabel = "";
              currentParsingStr = "";
            }
            waitForBoolean = false;
            break;
          default:
            if (waitForBoolean) {
              if (chunk[i] != ' ') currentParsingStr += chunk[i];
            } else if (!initialString)
              currentParsingStr += chunk[i];

            break;
        };
        currentIndex++;
      }
      // esp3d_log("%s", chunk);
    }
  } while (chunksize != 0 && !parsingDone);
  flashFs.close(prefsHandle);
  esp3d_log("Reading file done");
  if (valueSearched.length() > 0) {
    esp3d_log("%s = %s, as %s, at %d for %d bytes", entry,
              valueSearched.c_str(), valueIsString ? "String" : "Boolean",
              valueIndex - valueSearched.length() - 1, valueSearched.length());
  }
  return str.c_str();
}
bool ESP3DJsonSettings::writeString(ESP3DSettingIndex index,
                                    const char* byte_buffer) {
  return false;
}
