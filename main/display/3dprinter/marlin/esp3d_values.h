
/*
  esp3d_values.h -  values esp3d functions class

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

#include <list>
#include <string>
#if ESP3D_DISPLAY_FEATURE
#include "lvgl.h"
#endif  // ESP3D_DISPLAY_FEATURE

#ifdef __cplusplus
extern "C" {
#endif

// this list depend of target feature
enum class ESP3DValuesIndex : uint16_t { current_ip, unknown_index };

enum class ESP3DValuesType : uint8_t {
  unknown = 0,
  byte_t,
  integer_t,
  string_t,
  float_t,
};

struct ESP3DValuesDescription {
  ESP3DValuesIndex index = ESP3DValuesIndex::unknown_index;
  ESP3DValuesType type = ESP3DValuesType::unknown;
  size_t size = 0;
  std::string value = "";
#if ESP3D_DISPLAY_FEATURE
  lv_obj_t* label = nullptr;
#endif  // ESP3D_DISPLAY_FEATURE
};

class ESP3DValues final {
 public:
  ESP3DValues();
  ~ESP3DValues();
  const ESP3DValuesDescription* get_description(ESP3DValuesIndex index);
  uint8_t get_byte_value(ESP3DValuesIndex index);
  int get_integer_value(ESP3DValuesIndex index);
  const char* get_string_value(ESP3DValuesIndex index);
  float get_float_value(ESP3DValuesIndex index);
  bool set_byte_value(ESP3DValuesIndex index, uint8_t value);
  bool set_integer_value(ESP3DValuesIndex index, int value);
  bool set_string_value(ESP3DValuesIndex index, const char* value);
  bool set_float_value(ESP3DValuesIndex index, float value);

 private:
  std::list<ESP3DValuesDescription> _values;
};

extern ESP3DValues esp3dTftValues;

#ifdef __cplusplus
}  // extern "C"
#endif