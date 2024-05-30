/*
  main_screen.h - ESP3D screens styles definition

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

#include "esp3d_values.h"

namespace mainScreen {
void create();
bool extruder_0_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action);
bool extruder_1_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action);
bool bed_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action);
bool position_value_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action);
bool callback(ESP3DValuesIndex index, const char *value,
              ESP3DValuesCbAction action);
bool speed_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action);
bool job_status_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action);
void show_fan_controls(bool show);

}  // namespace mainScreen
