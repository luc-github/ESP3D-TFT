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
const char * esp3d_strings::str_trim(const char * str)
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
void  esp3d_strings::str_toUpperCase(std::string * str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::toupper);
}

//Lower case string
void  esp3d_strings::str_toLowerCase(std::string * str)
{
    std::transform(str->begin(), str->end(), str->begin(), ::tolower);
}

//helper to format size to readable string
const char* esp3d_strings::formatBytes (uint64_t bytes)
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

const char*  esp3d_strings::urlDecode(const char * text)
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

bool esp3d_strings::endsWith(const char * str, const char * endpart)
{
    if (!str || !endpart) {
        return false;
    }
    uint lenStr = strlen(str);
    uint lenEnd = strlen(endpart);
    if (lenEnd>lenStr) {
        return false;
    }
    for(uint i=0; i < lenEnd; i++) {
        if (str[lenStr-1-i]!= endpart[lenEnd-1-i]) {
            return false;
        }
    }
    return true;
}

bool esp3d_strings::startsWith(const char * str, const char * startPart)
{
    if (!str || !startPart) {
        return false;
    }
    uint lenStr = strlen(str);
    uint lenStart = strlen(startPart);
    if (lenStart>lenStr) {
        return false;
    }
    for(uint i=0; i < lenStart; i++) {
        if (str[i]!= startPart[i]) {
            return false;
        }
    }
    return true;
}

const char* esp3d_strings::getContentType (const char* filename)
{
    std::string file_name = filename;
    esp3d_strings::str_toLowerCase(&file_name);
    std::size_t found = file_name.find_last_of(".");
    if (found) {
        std::string extension = file_name.substr(found+1);
        if (extension =="htm")  {
            return "text/html";
        } else if (extension =="html")  {
            return "text/html";
        } else if (extension =="css") {
            return "text/css";
        } else if (extension =="js") {
            return "application/javascript";
        } else if (extension =="png") {
            return "image/png";
        } else if (extension =="gif") {
            return "image/gif";
        } else if (extension =="jpeg") {
            return "image/jpeg";
        } else if (extension =="jpg") {
            return "image/jpeg";
        } else if (extension =="ico") {
            return "image/x-icon";
        } else if (extension =="xml") {
            return "text/xml";
        } else if (extension =="pdf") {
            return "application/x-pdf";
        } else if (extension =="zip") {
            return "application/x-zip";
        } else if (extension =="gz") {
            return "application/x-gzip";
        } else if (extension =="txt") {
            return "text/plain";
        }
    }
    return "application/octet-stream";
}

int esp3d_strings::find(const char * str, const char *subStr, size_t start)
{
    if (!str ||!subStr ||start +strlen(subStr)>strlen(str)) {
        return -1;
    }
    for (int i=start; i<strlen(str); i++) {
        if (esp3d_strings::startsWith(&str[i], subStr)) {
            return i;
        }
    }
    return -1;
}