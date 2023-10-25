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
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#include "esp3d_fs_types.h"
#include "esp_vfs.h"

enum class ESP3DFsState : uint8_t {
  idle,
  busy,
  unknown,
};

class ESP3DFlash final {
 public:
  ESP3DFlash();
  bool begin();
  bool mount();
  void unmount();
  bool isMounted() { return _mounted; };
  const char *getFileSystemName();
  uint maxPathLength();
  bool getSpaceInfo(size_t *totalBytes = NULL, size_t *usedBytes = NULL,
                    size_t *freeBytes = NULL, bool refreshStats = false);
  ESP3DFileSystemType getFSType(const char *path = nullptr);
  bool accessFS(ESP3DFileSystemType FS = ESP3DFileSystemType::flash);
  void releaseFS(ESP3DFileSystemType FS = ESP3DFileSystemType::flash);
  bool format();
  const char *mount_point() { return "/fs"; }
  DIR *opendir(const char *dirpath);
  int closedir(DIR *dirp);
  int stat(const char *filepath, struct stat *entry_stat);
  bool exists(const char *path);
  bool remove(const char *path);
  bool mkdir(const char *path);
  bool rmdir(const char *path);
  bool rename(const char *oldpath, const char *newpath);
  struct dirent *readdir(DIR *dir);
  void rewinddir(DIR *dir);
  FILE *open(const char *filename, const char *mode);
  void close(FILE *fd);
  ESP3DFsState getState();
  ESP3DFsState setState(ESP3DFsState state);

 private:
  bool _mounted;
  bool _started;
  ESP3DFsState _state;
};

extern ESP3DFlash flashFs;