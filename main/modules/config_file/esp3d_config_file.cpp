/*
  esp3d_network
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

#include "esp3d_config_file.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_globalfs.h"

#define LINE_MAX_SIZE 255
#define SECTION_MAX_SIZE 30
#define KEY_MAX_SIZE 30
#define VALUE_MAX_SIZE 128

ESP3DConfigFile::ESP3DConfigFile(const char* path, processingFunction_t fn,
                                 const char* scrambledpath,
                                 const char** protectedkeys) {
  _filename = path;
  if (scrambledpath) {
    _scrambledFilename = scrambledpath;
  }
  _protectedkeys = protectedkeys;
  _pfunction = fn;
}

bool ESP3DConfigFile::processFile(const char* section_request,
                                  const char* key_request, char* value_found,
                                  size_t max_size) {
  bool res = true;
  if (!_filename.length()) {
    esp3d_log_e("No filename provided");
    return false;
  }
  FILE* rFile = globalFs.open(_filename.c_str(), "r");
  if (rFile) {
    bool processing = true;
    char line[LINE_MAX_SIZE + 1];
    char section[SECTION_MAX_SIZE + 1];  // system / network / services
    char key[KEY_MAX_SIZE + 1];
    uint8_t pos = 0;
    line[0] = '\0';
    section[0] = '\0';
    size_t readCount = 0;
    do {
      // to handle file without endline
      char c;
      readCount = fread(&c, 1, 1, rFile);
      if (readCount > 0) {
        if (!((c == '\n') || (c == '\r')) && (pos < (LINE_MAX_SIZE - 1))) {
          line[pos] = c;
          pos++;
        }
        if ((c == '\n') || (c == '\r') || !processing ||
            (pos == (LINE_MAX_SIZE - 1))) {
          line[pos] = '\0';
          char* stmp = trimSpaces(line);
          if (strlen(stmp) > 0) {
            // is comment ?
            if (!isComment(stmp)) {
              // is section ?
              if (isSection(stmp)) {
                strcpy(section, getSectionName(stmp));
              } else {
                // is key + value?
                if (isValue(stmp) && strlen(section) > 0) {
                  strcpy(key, getKeyName(stmp));
                  if (_pfunction) {
                    if (!_pfunction(section, key, getValue(stmp))) {
                      res = false;
                    }
                  } else {
                    if (section_request && key_request && value_found) {
                      if (strcmp(section, section_request) == 0 &&
                          strcmp(key, key_request) == 0) {
                        strncpy(value_found, getValue(stmp), max_size);
                        readCount = 0;
                        break;
                      }
                    }
                  }
                }
              }
            }
          }
          pos = 0;
          line[pos] = '\0';
        }
      }

    } while (readCount > 0);
    globalFs.close(rFile, _filename.c_str());
    return res;
  }

  esp3d_log_e("Cannot open ini file");
  return false;
}

bool ESP3DConfigFile::isComment(char* line) {
  if (strlen(line) > 0) {
    if ((line[0] == ';') || (line[0] == '#')) {
      return true;
    }
  }
  return false;
}

bool ESP3DConfigFile::isSection(char* line) {
  if (strlen(line) > 0) {
    if ((line[0] == '[') && (line[strlen(line) - 1] == ']')) {
      return true;
    }
  }
  return false;
}

bool ESP3DConfigFile::isValue(char* line) {
  if (strlen(line) > 3) {
    for (uint8_t i = 1; i < strlen(line) - 2; i++) {
      if (line[i] == '=') {
        return true;
      }
    }
  }
  return false;
}

char* ESP3DConfigFile::getSectionName(char* line) {
  line[strlen(line) - 1] = '\0';
  return trimSpaces(&line[1], SECTION_MAX_SIZE);
}

char* ESP3DConfigFile::getKeyName(char* line) {
  for (uint8_t i = 0; i < strlen(line); i++) {
    if (line[i] == '=') {
      line[i] = '\0';
      return trimSpaces(line, KEY_MAX_SIZE);
    }
  }
  return NULL;
}

char* ESP3DConfigFile::getValue(char* line) {
  char* startptr = line + strlen(line) + 1;
  while (*startptr == '\0') {
    startptr++;
  }
  return trimSpaces(startptr, VALUE_MAX_SIZE);
}

char* ESP3DConfigFile::trimSpaces(char* line, uint8_t maxsize) {
  char* endptr = line + strlen(line) - 1;
  char* startptr = line;
  while (endptr >= line && isspace(*endptr)) {
    *endptr-- = '\0';
  }
  endptr = line + strlen(line) - 1;
  while (endptr != startptr && isspace(*startptr)) {
    startptr++;
  }
  if ((maxsize > 0) && (strlen(startptr) > maxsize)) {
    startptr[maxsize] = '\0';
  }
  return startptr;
}

ESP3DConfigFile::~ESP3DConfigFile() {}

bool ESP3DConfigFile::isScrambleKey(const char* key, const char* str) {
  if (strlen(key) > strlen(str)) {
    return false;
  }
  for (uint8_t p = 0; p < strlen(str); p++) {
    if (p < strlen(key)) {
      if (std::toupper(key[p]) != std::toupper(str[p])) {
        return false;
      }
    } else {
      if (str[p] != ' ') {
        if (str[p] == '=') {
          return true;
        } else {
          return false;
        }
      }
    }
  }

  return false;
}

bool ESP3DConfigFile::revokeFile() {
  if (!_scrambledFilename.length()) {
    esp3d_log_e("No scrambled filename provided");
    return false;
  }
  // if CONFIG_FILE_OK already exists,  rename it to CONFIG_FILE_OKXX
  if (globalFs.exists(_scrambledFilename.c_str())) {
    std::string filename = _scrambledFilename;
    uint8_t n = 1;
    // find proper name
    while (globalFs.exists(filename.c_str())) {
      filename = _scrambledFilename + std::to_string(n++);
    }
    // rename CONFIG_FILE_OK to CONFIG_FILE_OKXX
    if (!globalFs.rename(_scrambledFilename.c_str(), filename.c_str())) {
      esp3d_log_e("Failed to rename %s", _scrambledFilename.c_str());
      // to avoid dead loop
      return false;
    }
  }

  FILE* wFile = globalFs.open(_scrambledFilename.c_str(), "w");
  FILE* rFile = globalFs.open(_filename.c_str(), "r");
  if (wFile && rFile) {
    char line[LINE_MAX_SIZE + 1];
    uint8_t pos = 0;
    line[0] = '\0';
    size_t readCount = 0;
    do {
      // to handle file without endline
      char c;
      readCount = fread(&c, 1, 1, rFile);
      if (readCount > 0) {
        if (!((c == '\n') || (c == '\r')) && (pos < (LINE_MAX_SIZE - 1))) {
          line[pos] = c;
          pos++;
        }
        if ((c == '\n') || (c == '\r') || (pos == (LINE_MAX_SIZE - 1))) {
          line[pos] = '\0';
          char* stmp = trimSpaces(line);
          if (strlen(stmp) > 0) {
            if (_protectedkeys && sizeof(_protectedkeys) > 0) {
              bool foundscramble = false;
              uint8_t size = sizeof(_protectedkeys) / sizeof(char*);
              for (uint8_t i = 0; (i < size) && !foundscramble; i++) {
                if (isScrambleKey(_protectedkeys[i], stmp)) {
                  strcpy(line, _protectedkeys[i]);
                  strcat(line, "=********");
                  stmp = line;
                  foundscramble = true;
                }
              }
            }
            if (fwrite(stmp, strlen(stmp), 1, wFile) != 1) {
              esp3d_log_e("Error cannot writing data on flash filesystem");
              globalFs.close(wFile, _scrambledFilename.c_str());
              globalFs.close(rFile, _filename.c_str());
              return false;
            }
            if (fwrite("\r\n", 2, 1, wFile) != 1) {
              esp3d_log_e("Error cannot writing data on flash filesystem");
              globalFs.close(wFile, _scrambledFilename.c_str());
              globalFs.close(rFile, _filename.c_str());
              return false;
            }
          }
          pos = 0;
          line[pos] = '\0';
        }
      }
    } while (readCount > 0);
    globalFs.close(wFile, _scrambledFilename.c_str());
    globalFs.close(rFile, _filename.c_str());
    if (!globalFs.remove(_filename.c_str())) {
      esp3d_log_e("Failed to remove %s", _filename.c_str());
      return false;
    }
    return true;
  }
  globalFs.close(wFile, _scrambledFilename.c_str());
  globalFs.close(rFile, _filename.c_str());
  esp3d_log_e("Cannot open / create revoked file");
  return false;
}