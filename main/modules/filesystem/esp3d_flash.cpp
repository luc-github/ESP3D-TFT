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

#include <stdio.h>
#include <string.h>
#include "esp3d_log.h"
#include "esp3d_flash.h"

ESP3D_FLASH flashFs;

ESP3D_FLASH::ESP3D_FLASH()
{
    _mounted = false;
    _started = false;
}

esp3d_fs_types ESP3D_FLASH::getFSType(const char * path)
{
    (void)path;
    return FS_FLASH;
}

bool  ESP3D_FLASH::accessFS(esp3d_fs_types FS)
{
    (void)FS;
    esp3d_log("Access SD");
    return _mounted;

}

void  ESP3D_FLASH::releaseFS(esp3d_fs_types FS)
{
    (void)FS;
    esp3d_log("Release SD");
}

