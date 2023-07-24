
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
#if ESP3D_DISPLAY_FEATURE
bool status_bar_cb(ESP3DValuesIndex index, const char *value,
                   ESP3DValuesCbAction action);
bool extruder_0_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action);
bool extruder_0_target_cb(ESP3DValuesIndex index, const char *value,
                          ESP3DValuesCbAction action);
#endif  // ESP3D_DISPLAY_FEATURE

bool ESP3DValues::intialize() {
  clear();
#if ESP3D_DISPLAY_FEATURE
  // status bar label
  _values.push_back({ESP3DValuesIndex::status_bar_label,
                     ESP3DValuesType::string_t, 200, std::string(""),
                     status_bar_cb});
#endif  // ESP3D_DISPLAY_FEATURE
  //  current ip
  _values.push_back({
      ESP3DValuesIndex::current_ip,
      ESP3DValuesType::string_t,
      16,  // size
      std::string("?"),
      nullptr,
  });

  //  ext 0 temperature
  _values.push_back({
      ESP3DValuesIndex::ext_0_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      extruder_0_value_cb,
  });

  //  ext 1 temperature
  _values.push_back({
      ESP3DValuesIndex::ext_1_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      extruder_0_target_cb,
  });

  //  bed temperature
  _values.push_back({
      ESP3DValuesIndex::bed_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      nullptr,
  });

  //  ext 0 target temperature
  _values.push_back({
      ESP3DValuesIndex::ext_0_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      nullptr,
  });

  //  ext 1 target temperature
  _values.push_back({
      ESP3DValuesIndex::ext_1_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      nullptr,
  });

  //  bed target temperature
  _values.push_back({
      ESP3DValuesIndex::bed_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      nullptr,
  });

  //  ext 0 fan speed
  _values.push_back({
      ESP3DValuesIndex::ext_0_fan_speed,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("100%"),
      nullptr,
  });

  //  ext 1 fan speed
  _values.push_back({
      ESP3DValuesIndex::ext_1_fan_speed,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("100%"),
      nullptr,
  });

  //  x position

  _values.push_back({
      ESP3DValuesIndex::x_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      nullptr,
  });

  //  y position
  _values.push_back({
      ESP3DValuesIndex::y_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      nullptr,
  });

  //  z position

  _values.push_back({
      ESP3DValuesIndex::z_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      nullptr,
  });

  return _values.size() != 0;
}
