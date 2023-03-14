/*
  esp_sd.cpp - ESP3D SD support class

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
#if ESP3D_SD_CARD_FEATURE
#include "esp3d_sd.h"

#include <stdio.h>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "sd_def.h"

Esp3dSd sd;

Esp3dSd::Esp3dSd() {
  _mounted = false;
  _started = false;
  _spi_speed_divider = 0;
  _state = Esp3dSdState::unknown;
}

esp3d_fs_types Esp3dSd::getFSType(const char* path) {
  (void)path;
  return FS_SD;
}

bool Esp3dSd::accessFS(esp3d_fs_types FS) {
  (void)FS;
  // if card is busy do not let another task access SD and so prevent a release
  if (getState() != Esp3dSdState::idle) {
    esp3d_log("SDCard not idle");
    return false;
  }
  esp3d_log("Access SD");
  _state = Esp3dSdState::busy;
  return true;
}

void Esp3dSd::releaseFS(esp3d_fs_types FS) {
  (void)FS;
  esp3d_log("Release SD");
  setState(Esp3dSdState::idle);
}

Esp3dSdState Esp3dSd::getState() {
  if (_state == Esp3dSdState::busy) {
    return _state;
  }
  mount();
  return _state;
};

#endif  // ESP3D_SD_CARD_FEATURE
