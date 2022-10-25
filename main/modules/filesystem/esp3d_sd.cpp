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

#include "sd_def.h"
#include "esp3d_sd.h"
#include <stdio.h>
#include <string.h>
#include "esp3d_log.h"

ESP3D_SD sd;

ESP3D_SD::ESP3D_SD()
{
    _mounted = false;
    _started = false;
    _spi_speed_divider = 0;
    _state = ESP3D_SDCARD_UNKNOWN;
}

esp3d_fs_types ESP3D_SD::getFSType(const char * path)
{
    (void)path;
    return FS_SD;
}

bool  ESP3D_SD::accessFS(esp3d_fs_types FS)
{
    (void)FS;
    //if card is busy do not let another task access SD and so prevent a release
    if (_state == ESP3D_SDCARD_BUSY) {
        esp3d_log( "SDCard Busy.");
        return false;
    }
    esp3d_log("Access SD");
    return true;

}

void  ESP3D_SD::releaseFS(esp3d_fs_types FS)
{
    (void)FS;
    esp3d_log("Release SD");
    setState(ESP3D_SDCARD_IDLE);
}


esp3d_sd_states ESP3D_SD::getState()
{
    if (_state==ESP3D_SDCARD_BUSY) {
        return _state;
    }
    mount();
    return _state;
};
