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
#include <algorithm>
#include <string.h>

//Trim string function
const char * str_trim(const char * str)
{
    static std::string s = str;
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
//Upper case string
void  str_toUpperCase(std::string * str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}

//helper to format size to readable string
const char* formatBytes (uint64_t bytes)
{
    static char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    int res = 0;
    if (bytes < 1024) {
        res = snprintf(buffer, sizeof(buffer), "%d B", (int)bytes);
    } else if (bytes < (1024 * 1024) ) {
        res =  snprintf(buffer, sizeof(buffer), "%.2f KB", ((float)(bytes / 1024.0)));
    } else if (bytes < (1024 * 1024 * 1024) ) {
        res =  snprintf(buffer, sizeof(buffer), "%.2f MB", ((float)(bytes / 1024.0 / 1024.0)));
    } else {
        res =  snprintf(buffer, sizeof(buffer), "%.2f GB", ((float)(bytes / 1024.0 / 1024.0 / 1024.0)));
    }
    if (res < 0) {
        strcpy(buffer, "? B");
    }
    return buffer;
}

const char*  urlDecode(const char * text)
{
    static char * decoded=nullptr;
    if (decoded) {
        free(decoded);
    }
    char temp[] = "0x00";
    unsigned int len = strlen(text);
    unsigned int i = 0;
    unsigned int p = 0;
    decoded = (char*)malloc(len +1);
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
                    decodedChar = encodedChar;    // normal ascii char
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