
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
#include "esp3d_version.h"

ESP3DValues esp3dTftValues;

// value of settings, storage values are all strings because it is easier to
// display

ESP3DValues::ESP3DValues() { intialize(); }

void ESP3DValues::clear() {
  for (auto element = _values.begin(); element != _values.end(); ++element) {
    if (element->callbackFn) {
      element->callbackFn(element->index, nullptr, ESP3DValuesCbAction::Clear);
    }
  }
  _values.clear();
}

ESP3DValues::~ESP3DValues() { clear(); }

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
      if (element->callbackFn) {
        element->callbackFn(element->index, value, ESP3DValuesCbAction::Update);
      }
      return true;
    }
  }
  return false;
}
bool ESP3DValues::set_float_value(ESP3DValuesIndex index, float value) {
  return false;
}