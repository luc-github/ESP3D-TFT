
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
bool extruder_1_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action);
bool bed_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action);
bool fan_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action);
bool speed_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action);
bool position_value_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action);
bool print_status_value_cb(ESP3DValuesIndex index, const char *value,
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
      extruder_1_value_cb,
  });

  //  bed temperature
  _values.push_back({
      ESP3DValuesIndex::bed_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      bed_value_cb,
  });

  //  ext 0 target temperature
  _values.push_back({
      ESP3DValuesIndex::ext_0_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      extruder_0_value_cb,
  });

  //  ext 1 target temperature
  _values.push_back({
      ESP3DValuesIndex::ext_1_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      extruder_1_value_cb,
  });

  //  bed target temperature
  _values.push_back({
      ESP3DValuesIndex::bed_target_temperature,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0.00"),
      bed_value_cb,
  });

  //  ext 0 fan
  _values.push_back({
      ESP3DValuesIndex::ext_0_fan,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("100"),
      fan_value_cb,
  });

  //  ext 1 fan
  _values.push_back({
      ESP3DValuesIndex::ext_1_fan,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("100"),
      fan_value_cb,
  });

  //
  _values.push_back({
      ESP3DValuesIndex::speed,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("100"),
      fan_value_cb,
  });

  //  x position

  _values.push_back({
      ESP3DValuesIndex::x_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      position_value_cb,
  });

  //  y position
  _values.push_back({
      ESP3DValuesIndex::y_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      position_value_cb,
  });

  //  z position

  _values.push_back({
      ESP3DValuesIndex::z_position,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("?"),
      position_value_cb,
  });

  //  print status
  _values.push_back({
      ESP3DValuesIndex::print_status,
      ESP3DValuesType::string_t,
      200,  // precision
      std::string("idle"),
      print_status_value_cb,
  });

  return _values.size() != 0;
}
