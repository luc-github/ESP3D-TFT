/*
  wifi_status_component.h - ESP3D screens styles definition

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

#include <lvgl.h>

#include "esp3d_values.h"

namespace wifiStatus {
extern lv_obj_t *wifi_status(lv_obj_t *parent, lv_obj_t *backbutton);

extern bool network_status_value_cb(ESP3DValuesIndex index, const char *value,
                                    ESP3DValuesCbAction action);
extern bool network_mode_value_cb(ESP3DValuesIndex index, const char *value,
                                  ESP3DValuesCbAction action);

}  // namespace wifiStatus
