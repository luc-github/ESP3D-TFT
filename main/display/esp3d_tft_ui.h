/*
  esp3d_tft_ui

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
#include <stdio.h>

#include "screens/esp3d_screen_type.h"

class ESP3DTftUi final {
 public:
  ESP3DTftUi();
  ~ESP3DTftUi();
  bool begin();
  void handle();
  bool end();
  void set_current_screen(ESP3DScreenType screen) { _current_screen = screen; }
  ESP3DScreenType get_current_screen() { return _current_screen; }

 private:
  ESP3DScreenType _current_screen = ESP3DScreenType::none;
  bool _started;
};

extern ESP3DTftUi esp3dTftui;
