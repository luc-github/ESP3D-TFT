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
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs.h"


class ESP3D_GLOBALFS final
{
public:
    ESP3D_GLOBALFS();
    bool begin();
    const char * getFileSystemName(char * path);
    uint maxPathLength(esp3d_fs_types fstype=FS_FLASH);
    bool getSpaceInfo(uint64_t * totalBytes=NULL,
                      uint64_t * usedBytes=NULL,
                      uint64_t * freeBytes=NULL,
                      const char *path=NULL,
                      bool refreshStats=false);
    esp3d_fs_types getFSType(const char * path=nullptr);
    bool  accessFS(const char* path);
    void  releaseFS(const char* path);
    DIR * opendir(const char * dirpath);
    int closedir(DIR *dirp);
    int stat(const char * filepath,  struct  stat * entry_stat);
    bool exists(const char* path);
    bool remove(const char *path);
    bool mkdir(const char *path);
    bool rmdir(const char *path);
    bool rename(const char *oldpath, const char *newpath);
    struct dirent * readdir(DIR *dirp);
    void rewinddir(DIR * dirp);
    FILE * open ( const char * filename, const char * mode );
    void close(FILE * fd, const char * filename);
private:
    struct dirent _rootEntry; //there no multiple access to root so 1 should be enough
    DIR _rootDir; //there no multiple access to root so 1 should be enough
};

extern ESP3D_GLOBALFS globalFs;