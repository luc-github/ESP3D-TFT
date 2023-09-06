
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
const char* ESP3DValues::get_string_value(ESP3DValuesIndex index) {
  const ESP3DValuesDescription* e = get_description(index);
  if (e == nullptr) return nullptr;
  return e->value.c_str();
  return nullptr;
}

bool ESP3DValues::set_string_value(ESP3DValuesIndex index, const char* value,
                                   ESP3DValuesCbAction action) {
  if (_values.size() == 0) {
    // No values list set  - service is ignored
    return true;
  }
  for (auto element = _values.begin(); element != _values.end(); ++element) {
    if (element->index == index) {
      element->value = value;
      esp3d_log("Setting String value %s for %d", value, (int)index);
      if (element->callbackFn) {
        element->callbackFn(element->index, value, action);
      }
      return true;
    }
  }
  esp3d_log_w("Cannot set String value %s for %d", value, (int)index);
  return false;
}
