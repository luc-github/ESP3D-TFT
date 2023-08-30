/*
message_box_component.h - ESP3D screens styles definition

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

enum class MsgBoxType : uint8_t {
  error = 0,
  information,
  confirmation,
};

namespace msgBox {
extern lv_obj_t *messageBox(lv_obj_t *container, MsgBoxType type,
                            const char *content);

extern lv_obj_t *confirmationBox(lv_obj_t *container, MsgBoxType type,
                                 const char *content, lv_event_cb_t event_cb);

}  // namespace msgBox
