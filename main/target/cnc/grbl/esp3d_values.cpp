
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
#include "components/status_bar_component.h"
#include "components/wifi_status_component.h"
#include "screens/main_screen.h"

#endif  // ESP3D_DISPLAY_FEATURE

bool ESP3DValues::intialize() {
  clear();
#if ESP3D_DISPLAY_FEATURE
  // status bar label
  _values.push_back({ESP3DValuesIndex::status_bar_label,
                     ESP3DValuesType::string_t, 200, std::string(""),
                     statusBar::callback});

  //  current ip
  _values.push_back({
      ESP3DValuesIndex::current_ip,
      ESP3DValuesType::string_t,
      16,  // size
      std::string("?"),
      nullptr,
  });

  //  x machine position
  _values.push_back({
      ESP3DValuesIndex::m_position_x,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      mainScreen::position_values,
  });

  //  y machine position
  _values.push_back({
      ESP3DValuesIndex::m_position_y,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  z machine position
  _values.push_back({
      ESP3DValuesIndex::m_position_z,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  a machine position
  _values.push_back({
      ESP3DValuesIndex::m_position_a,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  b machine position
  _values.push_back({
      ESP3DValuesIndex::m_position_b,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  c machine position

  _values.push_back({
      ESP3DValuesIndex::m_position_c,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  x work position
  _values.push_back({
      ESP3DValuesIndex::w_position_x,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  y work position
  _values.push_back({
      ESP3DValuesIndex::w_position_y,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  z work position
  _values.push_back({
      ESP3DValuesIndex::w_position_z,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  a work position
  _values.push_back({
      ESP3DValuesIndex::w_position_a,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  b work position
  _values.push_back({
      ESP3DValuesIndex::w_position_b,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  //  c work position
  _values.push_back({
      ESP3DValuesIndex::w_position_c,
      ESP3DValuesType::float_t,
      4,  // precision
      std::string("?"),
      nullptr,
  });

  // state
  _values.push_back({ESP3DValuesIndex::state, ESP3DValuesType::string_t,
                     10,  // precision
                     std::string("idle"), mainScreen::state_value_cb});

  //  state comment
  _values.push_back({
      ESP3DValuesIndex::state_comment,
      ESP3DValuesType::string_t,
      100,  // precision
      std::string(""),
      mainScreen::state_comment_value_cb,
  });

  //  print status
  _values.push_back({
      ESP3DValuesIndex::job_status,
      ESP3DValuesType::string_t,
      200,  // precision
      std::string("idle"),
      mainScreen::job_status_value_cb,
  });

  //  file path
  _values.push_back({
      ESP3DValuesIndex::file_path,
      ESP3DValuesType::string_t,
      255,  // size
      std::string(""),
      nullptr,
  });
  //  file name
  _values.push_back({
      ESP3DValuesIndex::file_name,
      ESP3DValuesType::string_t,
      255,  // size
      std::string(""),
      nullptr,
  });
#if ESP3D_WIFI_FEATURE
  //  network status
  _values.push_back({
      ESP3DValuesIndex::network_status,
      ESP3DValuesType::string_t,
      1,  // size
      std::string("?"),
      wifiStatus::network_status_cb,
  });
  //  network mode
  _values.push_back({
      ESP3DValuesIndex::network_mode,
      ESP3DValuesType::string_t,
      1,  // size
      std::string("?"),
      wifiStatus::network_mode_cb,
  });
#endif  // ESP3D_WIFI_FEATURE
  //  job progress
  _values.push_back({
      ESP3DValuesIndex::job_progress,
      ESP3DValuesType::float_t,
      2,  // precision
      std::string("0"),
      mainScreen::job_status_value_cb,
  });
  //  job elapsed duration
  _values.push_back({
      ESP3DValuesIndex::job_duration,
      ESP3DValuesType::integer_t,
      0,  // precision
      std::string("0"),
      nullptr,
  });

#endif  // ESP3D_DISPLAY_FEATURE
  return true;
}
