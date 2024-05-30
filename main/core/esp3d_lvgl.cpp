/*
  esp3d_hal helper functions

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
#if ESP3D_DISPLAY_FEATURE
#include "esp3d_lvgl.h"

bool lv_timer_is_valid(lv_timer_t *timer) {
  if (timer == NULL) return false;
  // parse all timers to check if the timer is valid
  lv_timer_t *t = lv_timer_get_next(NULL);
  while (t != NULL) {
    if (t == timer) return true;
    t = lv_timer_get_next(t);
  }
  return false;
}

#endif  // ESP3D_DISPLAY_FEATURE
