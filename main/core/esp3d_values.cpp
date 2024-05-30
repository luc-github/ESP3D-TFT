
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

#include <pthread.h>

#include <algorithm>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_version.h"

ESP3DValues esp3dTftValues;

// value of settings, storage values are all strings because it is easier to
// display

ESP3DValues::ESP3DValues() {
  intialize();
  if (pthread_mutex_init(&_mutex, NULL) != 0) {
    printf("\n mutex init failed\n");
  }
}

void ESP3DValues::clear() {
  for (auto element = _values.begin(); element != _values.end(); ++element) {
    if (element->callbackFn) {
      element->callbackFn(element->index, nullptr, ESP3DValuesCbAction::Clear);
    }
  }
  _values.clear();
}

ESP3DValues::~ESP3DValues() {
  clear();
  pthread_mutex_destroy(&_mutex);
}

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

void ESP3DValues::handle() {
  if (pthread_mutex_lock(&_mutex) == 0) {
    // check if there is something to do
    if (_updated_values_queue.size() > 0) {
      uint8_t nb = 0;
      // lets process 10 values max to avoid blocking the loop too long
      while (!_updated_values_queue.empty() && nb < 10) {
        ESP3DValuesData& element = _updated_values_queue.front();
        // update value and call the callback function if any
        element.description->value = element.value;
        esp3d_log("Setting String value %s for %d", element.value.c_str(),
                  (int)element.index);
        if (element.description->callbackFn) {
          element.description->callbackFn(element.index, element.value.c_str(),
                                          element.action);
        }
        // remove front element from queue
        _updated_values_queue.pop_front();
        nb++;
      }
    }
  }
  if (pthread_mutex_unlock(&_mutex) != 0) {
    esp3d_log_e("Cannot unlock mutex");
  }
}

bool ESP3DValues::set_string_value(ESP3DValuesIndex index, const char* value,
                                   ESP3DValuesCbAction action) {
  bool result = false;
  if (_values.size() == 0) {
    // No values list set  - service is ignored
    return true;
  }
  // use mutex to do successive calls and avoid any race condition
  if (pthread_mutex_lock(&_mutex) == 0) {
    // check if index is valid
    auto it = std::find_if(_values.begin(), _values.end(),
                           [index](const ESP3DValuesDescription& data) {
                             return data.index == index;
                           });
    // is it found ?
    if (it != _values.end()) {
      // yes found it, push value in queue to be processed later
      _updated_values_queue.emplace_back(ESP3DValuesData{
          std::string(value ? value : ""), action, index, &(*it)});
      result = true;
    } else {
      // not found - error
      esp3d_log_w("Cannot set String value %s for %d", value, (int)index);
    }
    if (pthread_mutex_unlock(&_mutex) != 0) {
      esp3d_log_e("Cannot unlock mutex");
    }
  } else {
    esp3d_log_e("Cannot lock mutex");
  }
  return result;
}
