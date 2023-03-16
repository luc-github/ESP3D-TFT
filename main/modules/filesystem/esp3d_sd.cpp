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

ESP3DSd sd;

ESP3DSd::ESP3DSd() {
  _mounted = false;
  _started = false;
  _spi_speed_divider = 0;
  _state = ESP3DSdState::unknown;
}

ESP3DFileSystemType ESP3DSd::getFSType(const char* path) {
  (void)path;
  return ESP3DFileSystemType::sd;
}

bool ESP3DSd::accessFS(ESP3DFileSystemType FS) {
  (void)FS;
  // if card is busy do not let another task access SD and so prevent a release
  if (getState() != ESP3DSdState::idle) {
    esp3d_log("SDCard not idle");
    return false;
  }
  esp3d_log("Access SD");
  _state = ESP3DSdState::busy;
  return true;
}

void ESP3DSd::releaseFS(ESP3DFileSystemType FS) {
  (void)FS;
  esp3d_log("Release SD");
  setState(ESP3DSdState::idle);
}

ESP3DSdState ESP3DSd::getState() {
  if (_state == ESP3DSdState::busy) {
    return _state;
  }
  mount();
  return _state;
};

#endif  // ESP3D_SD_CARD_FEATURE
