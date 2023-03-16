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
}

ESP3DFileSystemType ESP3DFlash::getFSType(const char* path) {
  (void)path;
  return ESP3DFileSystemType::flash;
}

bool ESP3DFlash::accessFS(ESP3DFileSystemType FS) {
  (void)FS;
  esp3d_log("Access FS");
  return _mounted;
}

void ESP3DFlash::releaseFS(ESP3DFileSystemType FS) {
  (void)FS;
  esp3d_log("Release FS");
}
