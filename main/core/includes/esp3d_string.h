/*
  esp3d_string helper functions

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

#pragma once
#include <stdio.h>

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

namespace esp3d_strings {
// TODO:
// some functions do internal change and others do copy of original string and
// modify it need to review for consistency  or allows both copy and insite of
// string TBD
std::string set_precision(std::string str_value, uint8_t precision);
const char* str_replace(const char* currentstr, const char* oldsubstr,
                        const char* newsubstr);
const char* str_trim(const char* str);
void str_toUpperCase(std::string* str);
void str_toLowerCase(std::string* str);
bool endsWith(const char* str, const char* endpart);
bool startsWith(const char* str, const char* startPart);
int find(const char* str, const char* subStr, size_t start = 0);
const char* formatBytes(uint64_t bytes);
const char* urlDecode(const char* text);
const char* getContentType(const char* filename);
}  // namespace esp3d_strings
#ifdef __cplusplus
}  // extern "C"
#endif