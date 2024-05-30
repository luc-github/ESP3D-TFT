/*
  esp3d-string helper functions

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

#include "esp3d_string.h"

#include <string.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "esp3d_hal.h"
#if ESP3D_WIFI_FEATURE
#include "network/esp3d_network.h"
#endif  // ESP3D_WIFI_FEATURE

// helper to format string float to readable string with precision
std::string esp3d_string::set_precision(std::string str_value,
                                        uint8_t precision) {
  static std::string tmp;
  tmp = str_value;
  double value = std::stod(str_value);
  double rounded =
      std::round(value * std::pow(10, precision)) / std::pow(10, precision);
  tmp = std::to_string(rounded);

  int pos = tmp.find(".");
  if (precision == 0) {
    tmp = tmp.substr(0, pos);
  } else {
    tmp = tmp.substr(0, pos + precision + 1);
  }

  return tmp;
}

// Replace all occurrences of oldsubstr by newsubstr and return new string
const char* esp3d_string::str_replace(const char* currentstr,
                                      const char* oldsubstr,
                                      const char* newsubstr) {
  static std::string resstr;
  if (!currentstr || !oldsubstr) {
    return nullptr;
  }
  resstr = currentstr;
  std::string oldstr = oldsubstr;
  std::string newstr = newsubstr ? newsubstr : "";

  size_t pos = 0;
  while ((pos = resstr.find(oldstr, pos)) != std::string::npos) {
    resstr.replace(pos, oldstr.length(), newstr);
    pos += newstr.length();
  }
  return resstr.c_str();
}

// Trim string function
const char* esp3d_string::str_trim(const char* str) {
  static std::string s;
  s = str;
  auto start = s.begin();
  while (start != s.end() && std::isspace(*start)) {
    start++;
  }
  auto end = s.end();
  do {
    end--;
  } while (std::distance(start, end) > 0 && std::isspace(*end));
  s = std::string(start, end + 1);
  return s.c_str();
}
// Upper case string
void esp3d_string::str_toUpperCase(std::string* str) {
  std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}

// Lower case string
void esp3d_string::str_toLowerCase(std::string* str) {
  std::transform(str->begin(), str->end(), str->begin(), ::tolower);
}

// helper to format size to readable string
const char* esp3d_string::formatBytes(uint64_t bytes) {
  static char buffer[32];
  memset(buffer, 0, sizeof(buffer));
  int res = 0;
  if (bytes < 1024) {
    res = snprintf(buffer, sizeof(buffer), "%d B", (int)bytes);
  } else if (bytes < (1024 * 1024)) {
    res =
        snprintf(buffer, sizeof(buffer), "%.2f KB", ((float)(bytes / 1024.0)));
  } else if (bytes < (1024 * 1024 * 1024)) {
    res = snprintf(buffer, sizeof(buffer), "%.2f MB",
                   ((float)(bytes / 1024.0 / 1024.0)));
  } else {
    res = snprintf(buffer, sizeof(buffer), "%.2f GB",
                   ((float)(bytes / 1024.0 / 1024.0 / 1024.0)));
  }
  if (res < 0) {
    strcpy(buffer, "? B");
  }
  return buffer;
}

const char* esp3d_string::urlDecode(const char* text) {
  static char* decoded = nullptr;
  if (decoded) {
    free(decoded);
  }
  char temp[] = "0x00";
  unsigned int len = strlen(text);
  unsigned int i = 0;
  unsigned int p = 0;
  decoded = (char*)malloc(len + 1);
  if (decoded) {
    while (i < len) {
      char decodedChar;
      char encodedChar = text[i++];
      if ((encodedChar == '%') && (i + 1 < len)) {
        temp[2] = text[i++];
        temp[3] = text[i++];
        decodedChar = strtol(temp, NULL, 16);
      } else {
        if (encodedChar == '+') {
          decodedChar = ' ';
        } else {
          decodedChar = encodedChar;  // normal ascii char
        }
      }
      decoded[p++] = decodedChar;
    }
    decoded[p] = 0x0;
    return decoded;
  } else {
    return nullptr;
  }
}

bool esp3d_string::endsWith(const char* str, const char* endpart) {
  if (!str || !endpart) {
    return false;
  }
  uint lenStr = strlen(str);
  uint lenEnd = strlen(endpart);
  if (lenEnd > lenStr) {
    return false;
  }
  for (uint i = 0; i < lenEnd; i++) {
    if (str[lenStr - 1 - i] != endpart[lenEnd - 1 - i]) {
      return false;
    }
  }
  return true;
}

bool esp3d_string::startsWith(const char* str, const char* startPart) {
  if (!str || !startPart) {
    return false;
  }
  uint lenStr = strlen(str);
  uint lenStart = strlen(startPart);
  if (lenStart > lenStr) {
    return false;
  }
  for (uint i = 0; i < lenStart; i++) {
    if (str[i] != startPart[i]) {
      return false;
    }
  }
  return true;
}

const char* esp3d_string::getContentType(const char* filename) {
  std::string file_name;
  file_name = filename;
  esp3d_string::str_toLowerCase(&file_name);
  std::size_t found = file_name.find_last_of(".");
  if (found) {
    std::string extension = file_name.substr(found + 1);
    if (extension == "htm") {
      return "text/html";
    } else if (extension == "html") {
      return "text/html";
    } else if (extension == "css") {
      return "text/css";
    } else if (extension == "js") {
      return "application/javascript";
    } else if (extension == "png") {
      return "image/png";
    } else if (extension == "gif") {
      return "image/gif";
    } else if (extension == "jpeg") {
      return "image/jpeg";
    } else if (extension == "jpg") {
      return "image/jpeg";
    } else if (extension == "ico") {
      return "image/x-icon";
    } else if (extension == "xml") {
      return "text/xml";
    } else if (extension == "pdf") {
      return "application/x-pdf";
    } else if (extension == "zip") {
      return "application/x-zip";
    } else if (extension == "gz") {
      return "application/x-gzip";
    } else if (extension == "txt") {
      return "text/plain";
    } else if (extension == "gco" || extension == "gcode") {
      return "text/plain";
    }
  }
  return "application/octet-stream";
}

int esp3d_string::find(const char* str, const char* subStr, size_t start) {
  if (!str || !subStr || start + strlen(subStr) > strlen(str)) {
    return -1;
  }
  for (int i = start; i < strlen(str); i++) {
    if (esp3d_string::startsWith(&str[i], subStr)) {
      return i;
    }
  }
  return -1;
}

int esp3d_string::rfind(const char* str, const char* subStr, int start) {
  if (!str || !subStr || strlen(subStr) > strlen(str)) {
    return -1;
  }
  if (start == -1) start = strlen(str) - 1;
  for (int i = start; i >= 0; i--) {
    if (esp3d_string::startsWith(&str[i], subStr)) {
      return i;
    }
  }
  return -1;
}

const char* esp3d_string::getPathFromString(const char* str) {
  static std::string path;
  path = str;
  int pos = path.find_last_of("/");
  if (pos != -1) {
    path = path.substr(0, pos);
  }
  return path.c_str();
}

const char* esp3d_string::getFilenameFromString(const char* str) {
  static std::string filename;
  filename = str;
  int pos = filename.find_last_of("/");
  if (pos != -1) {
    if (pos + 1 < filename.length()) {  // check if not last char
      filename = filename.substr(pos + 1);
    } else {  // last char is / so remove it and get dirname
      filename[0] = '\0';
      pos = filename.find_last_of("/");
      if (pos != -1) {
        filename = filename.substr(pos + 1);
      }
    }
  }
  return filename.c_str();
}

const char* esp3d_string::getTimeString(time_t time, bool isGMT) {
  static char buffer[40];
  memset(buffer, 0, sizeof(buffer));
  struct tm* tm_info;
  struct tm tmstruct;

  if (isGMT) {
    // convert to GMT time
    tm_info = gmtime_r(&time, &tmstruct);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
  } else {
    // convert to local time
    tm_info = localtime_r(&time, &tmstruct);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm_info);
  }
  if (!isGMT) {
#if ESP3D_TIMESTAMP_FEATURE
    // if time zone is set add it
    strcat(buffer, esp3dTimeService.getTimeZone());
#else
    // add Z to indicate UTC time because no time zone is set
    strcat(buffer, "Z");
#endif  // ESP3D_TIMESTAMP_FEATURE
  }
  return buffer;
}

// Update hash function used by generateUUID
void update_hash(uint8_t* data, size_t len, uint8_t* hash_buffer,
                 uint8_t hash_size) {
  static bool reverse = false;
  reverse = !reverse;
  int start_index = reverse ? hash_size : 0;
  for (int i = 0; i < hash_size; i++) {
    int idx =
        reverse ? (start_index - i) % hash_size : (start_index + i) % hash_size;
    if (i >= len) {
      hash_buffer[idx] ^= rand() % 253 + 1;
    } else {
      hash_buffer[idx] ^= data[i];
    }
  }
}

const char* esp3d_string::generateUUID(const char* seed) {
  static std::string token;
  std::string tmp;
  uint8_t hash_buffer[16];
  memset(hash_buffer, 0, 16);
  if (!seed) {
    tmp = "ESP3D ecosystem";
  } else {
    tmp = seed;
  }
  // init random seed
  srand(time(NULL));
  // Use seed
  update_hash((uint8_t*)tmp.c_str(), tmp.length(), hash_buffer, 16);

  // use epoch time
  uint64_t millisec = esp3d_hal::millis();
  update_hash((uint8_t*)&millisec, sizeof(millisec), hash_buffer, 16);

  // use current time
  time_t now;
  time(&now);
  update_hash((uint8_t*)&now, sizeof(now), hash_buffer, 16);

  tmp = "";
  // now hash all the buffer
  for (int i = 0; i < 16; i++) {
    char hex[3];
    sprintf(hex, "%02x", hash_buffer[i]);
    tmp += hex;
  }

  // format the uuid on 36 chars
  token = tmp.substr(0, 8) + "-";
  token += tmp.substr(8, 4) + "-";
  token += tmp.substr(12, 4) + "-";
  token += tmp.substr(16, 4) + "-";
  token += &tmp[20];

  return token.c_str();
}

const char* esp3d_string::expandString(const char* s, bool formatspace) {
  static std::string tmp;
  tmp = s;
  if (tmp.find("%") != std::string::npos) {
#if ESP3D_WIFI_FEATURE
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_IP%",
                                    esp3dNetwork.getLocalIpString());
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_NAME%",
                                    esp3dNetwork.getHostName());
#else
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_IP%", "???");
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_NAME%", "???");
#endif  // ESP3D_WIFI_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
    std::string dt = esp3dTimeService.getCurrentTime();
    if (formatspace) {
      dt = esp3d_string::str_replace(dt.c_str(), " ", "\\ ");
    }
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_DATETIME%", dt.c_str());
#else
    tmp = esp3d_string::str_replace(tmp.c_str(), "%ESP_DATETIME%", "???");
#endif  // ESP3D_TIMESTAMP_FEATURE
  }
  return tmp.c_str();
}