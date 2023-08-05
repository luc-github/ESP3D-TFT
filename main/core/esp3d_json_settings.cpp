
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

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_flash.h"

#define PREFERENCES_JSON_FILE "preferences.json"
#define PREFERENCES_TMP_FILE "preferences.tmp"
#define CHUNK_BUFFER_SIZE 255
ESP3DJsonSettings esp3dTftJsonSettings;

ESP3DJsonSettings::ESP3DJsonSettings() {}
ESP3DJsonSettings::~ESP3DJsonSettings() {}

ESP3DParseError ESP3DJsonSettings::parse(const char* file_name,
                                         const char* section,
                                         const char* entry) {
  size_t chunksize;
  char chunk[CHUNK_BUFFER_SIZE + 1];
  std::string currentLabel = "";
  std::string currentSection = "";
  bool initialString = true;
  bool waitForBoolean = false;
  uint deepthLevel = 0;
  int currentIndex = 0;
  _valueIndex = 0;
  _value = "";
  _is_value_str = false;
  std::string currentParsingStr = "";
  ESP3DParseError ret = ESP3DParseError::success;
  bool parsingDone = false;
  static std::string str;
  if (flashFs.accessFS()) {
    if (!flashFs.exists(file_name)) {
      esp3d_log_e("File %s does not exists", file_name);
      ret = ESP3DParseError::file_not_present;
    }
    if (ret == ESP3DParseError::success) {
      FILE* prefsHandle = flashFs.open(file_name, "rb");
      if (prefsHandle == NULL) {
        esp3d_log_e("Failed to open %s", file_name);
        flashFs.releaseFS();
        ret = ESP3DParseError::failed_opening_file;
      }
      if (ret == ESP3DParseError::success) {
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
                    // esp3d_log("Entering in section '%s'",
                    // currentLabel.c_str());
                    currentSection = currentLabel;
                    if (currentSection == section) {
                      deepthLevel = 0;
                    }
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
                    if (key == entry &&
                        (currentSection == section || deepthLevel == 0)) {
                      _is_value_str = false;
                      parsingDone = true;
                      _value = value;
                      _valueIndex = currentIndex - value.length();
                      // esp3d_log("Found");
                    }
                    // esp3d_log("Setting %s = %s", key.c_str(), value.c_str());
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
                      if (key == entry &&
                          (currentSection == section || deepthLevel == 0)) {
                        _is_value_str = true;
                        parsingDone = true;
                        _valueIndex = currentIndex - value.length();
                        _value = value;
                        // esp3d_log("Found at level %d", deepthLevel);
                      }
                      // esp3d_log("Setting %s = %s", key.c_str(),
                      // value.c_str());
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
                    if (key == entry &&
                        (currentSection == section || deepthLevel == 0)) {
                      _is_value_str = false;
                      parsingDone = true;
                      _valueIndex = currentIndex - value.length();
                      _value = value;
                      // esp3d_log("Found");
                    }
                    // esp3d_log("Setting %s = %s", key.c_str(), value.c_str());
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
          esp3d_hal::wait(2);
        } while (chunksize != 0 && !parsingDone);
      }
      flashFs.close(prefsHandle);
      esp3d_log("Reading file done");
      if (_value.length() > 0) {
        esp3d_log("%s = %s, as %s, at %d for %d bytes", entry, _value.c_str(),
                  _is_value_str ? "String" : "Boolean",
                  _valueIndex - _value.length() - 1, _value.length());
      }
      flashFs.releaseFS();
      ret = ESP3DParseError::success;
    }
    flashFs.releaseFS();
  } else {
    ret = ESP3DParseError::not_found;
  }
  return ret;
}

const char* ESP3DJsonSettings::readString(const char* section,
                                          const char* entry, bool* haserror) {
  ESP3DParseError res = parse(PREFERENCES_JSON_FILE, section, entry);
  if (haserror) {
    if (res == ESP3DParseError::success) {
      *haserror = false;
    } else {
      *haserror = true;
    }
  }
  return _value.c_str();
}
bool ESP3DJsonSettings::writeString(const char* section, const char* entry,
                                    const char* value) {
  ESP3DParseError res = parse(PREFERENCES_JSON_FILE, section, entry);

  bool success = false;
  if (res == ESP3DParseError::success) {
    esp3d_log("Found %s at %d", _value.c_str(), _valueIndex);
    if (flashFs.accessFS()) {
      bool has_error = false;
      FILE* prefsHandleTmp = flashFs.open(PREFERENCES_TMP_FILE, "wb");
      FILE* prefsHandleJson = flashFs.open(PREFERENCES_JSON_FILE, "rb");
      if (prefsHandleTmp && prefsHandleJson) {
        size_t chunksize;
        char chunk[CHUNK_BUFFER_SIZE + 1];
        size_t chunk_max_size = CHUNK_BUFFER_SIZE;
        int count = _valueIndex / CHUNK_BUFFER_SIZE;
        int offset = _valueIndex % CHUNK_BUFFER_SIZE;
        // esp3d_log("Do %d loop and an offset of %d bytes", count, offset);
        //  do part before position
        for (int p = 0; p < count; p++) {
          chunksize = fread(chunk, 1, chunk_max_size, prefsHandleJson);
          if (chunksize > 0) {
            if (0 == fwrite(chunk, 1, chunksize, prefsHandleTmp)) {
              has_error = true;
              esp3d_log_e("Error writing file");
            }
          } else {
            has_error = true;
            esp3d_log_e("Error reading file");
          }
          esp3d_hal::wait(2);
        }
        // write the offset difference
        if (!has_error) {
          chunk_max_size = chunksize = offset;
          fread(chunk, 1, chunk_max_size, prefsHandleJson);
          if (chunksize > 0) {
            if (0 == fwrite(chunk, 1, chunksize, prefsHandleTmp)) {
              has_error = true;
              esp3d_log_e("Error writing file");
            }
          } else {
            has_error = true;
            esp3d_log_e("Error reading file");
          }
        }
        // skip existing value length
        if (_value.length() > 0) {
          if (!has_error) {
            chunk_max_size = _value.length();
            chunksize = fread(chunk, 1, chunk_max_size, prefsHandleJson);
            if (chunksize <= 0) {
              has_error = true;
              esp3d_log_e("Error reading file");
            }
          }
        }
        if (strlen(value) > 0) {
          // Write new value
          if (!has_error) {
            if (0 == fwrite(value, strlen(value), 1, prefsHandleTmp)) {
              has_error = true;
              esp3d_log_e("Error writing file");
            }
          }
        }
        if (!has_error) {
          // do part after value writen
          chunk_max_size = CHUNK_BUFFER_SIZE;
          do {
            chunksize = fread(chunk, 1, chunk_max_size, prefsHandleJson);
            if (chunksize > 0) {
              // esp3d_log("Do final read of %d bytes", chunksize);
              if (0 == fwrite(chunk, 1, chunksize, prefsHandleTmp)) {
                has_error = true;
                esp3d_log_e("Error writing file");
              }
            }
            esp3d_hal::wait(2);
          } while (chunksize != 0 && !has_error);
        }
      } else {
        esp3d_log_e("Error opening file");
        has_error = true;
      }
      if (prefsHandleTmp) flashFs.close(prefsHandleTmp);
      if (prefsHandleJson) flashFs.close(prefsHandleJson);
      if (!has_error) {
        flashFs.remove(PREFERENCES_JSON_FILE);
        esp3d_hal::wait(2);
        flashFs.rename(PREFERENCES_TMP_FILE, PREFERENCES_JSON_FILE);
      }
      flashFs.releaseFS();
      success = !has_error;
    }
  } else if (res == ESP3DParseError::file_not_present) {
    success = false;
    std::string fileContent = "{\n\"settings\" : {\n\"";
    fileContent += entry;
    fileContent += "\" : \"";
    fileContent += value;
    fileContent += "\"\n}\n}";
    if (flashFs.accessFS()) {
      FILE* prefsHandleJson = flashFs.open(PREFERENCES_JSON_FILE, "wb");
      if (prefsHandleJson) {
        if (0 != fwrite(fileContent.c_str(), fileContent.length(), 1,
                        prefsHandleJson)) {
          esp3d_log("Create new file");
          success = true;
        }
        flashFs.close(prefsHandleJson);
      }
      flashFs.releaseFS();
      if (!success) {
        esp3d_log_e("Error creating file");
      }
    }
  } else {
    esp3d_log_e("Error accessing Fs");
    success = false;
  }
  return success;
}
