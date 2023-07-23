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

#include "esp3d_flash.h"

#include <stdio.h>
#include <string.h>

#include "esp3d_log.h"

ESP3DFlash flashFs;

ESP3DFlash::ESP3DFlash() {
  _mounted = false;
  _started = false;
  _state = ESP3DFsState::unknown;
}

ESP3DFileSystemType ESP3DFlash::getFSType(const char* path) {
  (void)path;
  return ESP3DFileSystemType::flash;
}

bool ESP3DFlash::accessFS(ESP3DFileSystemType FS) {
  (void)FS;
  esp3d_log("Access FS");
  if (_mounted) {
    setState(ESP3DFsState::busy);
  } else {
    setState(ESP3DFsState::unknown);
  }
  return _mounted;
}

void ESP3DFlash::releaseFS(ESP3DFileSystemType FS) {
  (void)FS;
  esp3d_log("Release FS");
  setState(ESP3DFsState::idle);
}

ESP3DFsState ESP3DFlash::getState() {
  if (!_mounted) setState(ESP3DFsState::unknown);
  return _state;
};

ESP3DFsState ESP3DFlash::setState(ESP3DFsState state) {
  _state = state;
#if ESP3D_TFT_LOG
  switch (_state) {
    case ESP3DFsState::idle:
      esp3d_log("FS Idle");
      break;
    case ESP3DFsState::busy:
      esp3d_log("FS busy");
      break;
    case ESP3DFsState::unknown:
      esp3d_log("FS unknown");
      break;
    default:
      esp3d_log("FS unknown state");
      break;
  }
#endif
  return _state;
}