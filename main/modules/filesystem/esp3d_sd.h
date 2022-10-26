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
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "ff.h"

#define ESP3D_SD_FS_HEADER "/sd/"
#define MAX_SD_PATH 255

typedef enum {
    ESP3D_SDCARD_IDLE,
    ESP3D_SDCARD_NOT_PRESENT,
    ESP3D_SDCARD_BUSY,
    ESP3D_SDCARD_UNKNOWN,
} esp3d_sd_states;

class ESP3D_SD final
{
public:
    ESP3D_SD();
    bool begin();
    bool mount();
    void unmount();
    bool isMounted()
    {
        return _mounted;
    };
    uint8_t getSPISpeedDivider()
    {
        return _spi_speed_divider;
    }
    void setSPISpeedDivider(uint8_t speeddivider)
    {
        _spi_speed_divider = speeddivider;
    }
    const char * getFileSystemName();
    uint maxPathLength();
    bool getSpaceInfo(uint64_t * totalBytes=NULL,
                      uint64_t * usedBytes=NULL,
                      uint64_t * freeBytes=NULL,
                      bool refreshStats=false);

    esp3d_sd_states getState();
    esp3d_sd_states setState(esp3d_sd_states state)
    {
        _state=state;
        return _state;
    }
    esp3d_fs_types getFSType(const char * path=nullptr);
    bool  accessFS(esp3d_fs_types FS=FS_SD);
    void  releaseFS(esp3d_fs_types FS=FS_SD);
    const char* mount_point()
    {
        return "/sd";
    }
    DIR * opendir(const char * dirpath);
    int closedir(DIR *dirp);
    int stat(const char * filepath,  struct  stat * entry_stat);
    bool exists(const char* path);
    bool remove(const char *path);
    bool mkdir(const char *path);
    bool rmdir(const char *path);
    bool rename(const char *oldpath, const char *newpath);
    struct dirent * readdir(DIR *dir);
    void rewinddir(DIR * dir);
    FILE * open ( const char * filename, const char * mode );
    void close(FILE * fd);
    /**
    void handle();
    void end();
    FILE open(const char* path, uint8_t mode = ESP3D_FILE_READ);

    void closeAll();
    */
private:
    bool _mounted;
    bool _started;
    esp3d_sd_states _state;
    uint8_t _spi_speed_divider;
};

extern ESP3D_SD sd;