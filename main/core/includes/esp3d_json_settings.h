
/*
  esp3d_json_settings.h -  settings esp3d functions class

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

#pragma once
#include <stdio.h>

#include "esp3d_settings.h"
#include "esp3d_string.h"

#ifdef __cplusplus
extern "C" {
#endif

enum class ESP3DParseError : uint8_t {
  no_error,
  failed_opening_file,
  success,
  save_failed,
  not_found,
  file_not_present
};

class ESP3DJsonSettings final {
 public:
  ESP3DJsonSettings();
  ~ESP3DJsonSettings();

  const char *readString(const char *section, const char *entry,
                         bool *haserror = NULL);
  bool writeString(const char *section, const char *entry, const char *value);

 private:
  ESP3DParseError parse(const char *file_name, const char *section,
                        const char *entry);
  std::string _value;
  bool _is_value_str;
  int _valueIndex;
};

extern ESP3DJsonSettings esp3dTftJsonSettings;

#ifdef __cplusplus
}  // extern "C"
#endif