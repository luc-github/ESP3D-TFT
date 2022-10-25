/*
  esp_sd.h - ESP3D SD support class

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

#pragma once
#include <stdio.h>
#include "esp3d_fs_types.h"

#define ESP3D_FLASH_FS_HEADER "/fs/"
#define MAX_FLASH_PATH 255

class ESP3D_FLASH final
{
public:
    ESP3D_FLASH();
    bool begin();
    bool mount();
    void unmount();
    bool isMounted()
    {
        return _mounted;
    };
    const char * getFileSystemName();
    uint maxPathLength();
    bool getSpaceInfo(size_t * totalBytes=NULL,
                      size_t * usedBytes=NULL,
                      size_t * freeBytes=NULL,
                      bool refreshStats=false);
    esp3d_fs_types getFSType(const char * path=nullptr);
    bool  accessFS(esp3d_fs_types FS=FS_FLASH);
    void  releaseFS(esp3d_fs_types FS=FS_FLASH);
    const char * getFullPath(const char * path=nullptr);
    /**
    void handle();
    void end();
    void refreshStats(bool force = false);*/
    bool format();
    /*
    FILE open(const char* path, uint8_t mode = ESP3D_FILE_READ);
    bool exists(const char* path);
    bool remove(const char *path);
    bool mkdir(const char *path);
    bool rmdir(const char *path);
    bool rename(const char *oldpath, const char *newpath);
    void closeAll();
    */
private:
    bool _mounted;
    bool _started;
};

extern ESP3D_FLASH flashFs;