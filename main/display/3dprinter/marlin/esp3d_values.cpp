
/*
  esp3d_values.cpp -  values esp3d functions class

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
#include "esp3d_values.h"

#include "esp3d_log.h"
#include "esp3d_string.h"

ESP3DValues esp3dTftValues;

// value of settings, storage values are all strings because it is easier to
// display

ESP3DValues::ESP3DValues() {
  _values.clear();
  _values.push_back(
      {ESP3DValuesIndex::current_ip, ESP3DValuesType::string_t, 16, ""});
}
ESP3DValues::~ESP3DValues() { _values.clear(); }
const ESP3DValuesDescription* ESP3DValues::get_description(
    ESP3DValuesIndex index) {
  for (auto element = _values.begin(); element != _values.end(); ++element) {
    if (element->index == index) return &(*element);
  }
  return nullptr;
}
uint8_t ESP3DValues::get_byte_value(ESP3DValuesIndex index) { return 0; }
int ESP3DValues::get_integer_value(ESP3DValuesIndex index) { return 0; }
const char* ESP3DValues::get_string_value(ESP3DValuesIndex index) {
  const ESP3DValuesDescription* e = get_description(index);
  if (e == nullptr) return nullptr;
  return e->value.c_str();
  return nullptr;
}
float ESP3DValues::ESP3DValues::get_float_value(ESP3DValuesIndex index) {
  return 0.0;
}
bool ESP3DValues::set_byte_value(ESP3DValuesIndex index, uint8_t value) {
  return false;
}
bool ESP3DValues::set_integer_value(ESP3DValuesIndex index, int value) {
  return false;
}
bool ESP3DValues::set_string_value(ESP3DValuesIndex index, const char* value) {
  for (auto element = _values.begin(); element != _values.end(); ++element) {
    if (element->index == index) {
      element->value = value;
#if ESP3D_DISPLAY_FEATURE
      if (element->label) {
        lv_label_set_text(element->label, value);
      }
#endif  // ESP3D_DISPLAY_FEATURE
      return true;
    }
  }
  return false;
}
bool ESP3DValues::set_float_value(ESP3DValuesIndex index, float value) {
  return false;
}