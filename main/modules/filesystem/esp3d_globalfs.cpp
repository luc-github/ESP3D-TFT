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

#include "esp3d_globalfs.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cstring>
#include <string>

#include "esp3d_flash.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"

#if ESP3D_SD_CARD_FEATURE
#include "esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

ESP3DGlobalFileSystem globalFs;
#define GLOBAL_ROOT_DIR_ID 8888
#define GLOBAL_FLASH_DIR_ID 1111
#if ESP3D_SD_CARD_FEATURE
#define GLOBAL_SD_DIR_ID 2222
#endif  // ESP3D_SD_CARD_FEATURE

const char *rootDirsHeaders[] = {ESP3D_FLASH_FS_HEADER
#if ESP3D_SD_CARD_FEATURE
                                 ,
                                 ESP3D_SD_FS_HEADER
#endif  // ESP3D_SD_CARD_FEATURE
};
uint8_t rootDirsHeadersSize = sizeof(rootDirsHeaders) / sizeof(char *);
const ESP3DFileSystemType rootDirsFSType[] = {ESP3DFileSystemType::flash
#if ESP3D_SD_CARD_FEATURE
                                              ,
                                              ESP3DFileSystemType::sd
#endif  // ESP3D_SD_CARD_FEATURE
};

ESP3DGlobalFileSystem::ESP3DGlobalFileSystem() {
  _rootDir.dd_vfs_idx = (uint16_t)-1;
  _rootDir.dd_rsv = GLOBAL_ROOT_DIR_ID;
  rewinddir(&_rootDir);
}

const char *ESP3DGlobalFileSystem::mount_point(ESP3DFileSystemType fstype) {
  switch (fstype) {
    case ESP3DFileSystemType::root:
      return "/";
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.mount_point();
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.mount_point();
    default:
      break;
  }
  return "/";
}

ESP3DFileSystemType ESP3DGlobalFileSystem::getFSType(const char *path) {
  if (!path || strlen(path) == 0) {
    return ESP3DFileSystemType::unknown;
  }
  if (strlen(path) == 1 && path[0] == '/') {
    return ESP3DFileSystemType::root;
  }
  std::string fullPath;
  if (path[0] != '/') {
    fullPath = "/";
    fullPath += path;
  } else {
    fullPath = path;
  }
  fullPath += "/";  // to handle /sd and /fs

  uint sizePath = fullPath.length();
  for (uint j = 0; j < rootDirsHeadersSize; j++) {
    uint sizeHeader = strlen(rootDirsHeaders[j]);
    for (uint i = 0; i < sizeHeader && i < sizePath; i++) {
      if (fullPath[i] != rootDirsHeaders[j][i]) {
        break;
      } else {
        if (i == sizeHeader - 1) {
          return rootDirsFSType[j];
        }
      }
    }
  }
  return ESP3DFileSystemType::unknown;
}

bool ESP3DGlobalFileSystem::accessFS(const char *path) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      return true;
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.accessFS();
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.accessFS();
    default:
      break;
  }
  return false;
}

void ESP3DGlobalFileSystem::releaseFS(const char *path) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      sd.releaseFS();
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      flashFs.releaseFS();
      break;
    default:
      break;
  }
}

bool ESP3DGlobalFileSystem::begin() { return true; }

const char *ESP3DGlobalFileSystem::getFileSystemName(char *path) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      return "Global";
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.getFileSystemName();
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.getFileSystemName();
    default:
      break;
  }
  return "Unknown";
}
uint ESP3DGlobalFileSystem::maxPathLength(ESP3DFileSystemType fstype) {
  switch (fstype) {
    case ESP3DFileSystemType::root:
      return 0;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.maxPathLength();
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.maxPathLength();
    default:
      break;
  }
  return false;
}
bool ESP3DGlobalFileSystem::getSpaceInfo(uint64_t *totalBytes,
                                         uint64_t *usedBytes,
                                         uint64_t *freeBytes, const char *path,
                                         bool refreshStats) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      return false;
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.getSpaceInfo(totalBytes, usedBytes, freeBytes, refreshStats);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash: {
      size_t total_Bytes;
      size_t used_Bytes;
      size_t free_Bytes;
      bool res = flashFs.getSpaceInfo(
          totalBytes ? &total_Bytes : NULL, usedBytes ? &used_Bytes : NULL,
          freeBytes ? &free_Bytes : NULL, refreshStats);
      if (res) {
        if (totalBytes) {
          *totalBytes = total_Bytes;
        }
        if (usedBytes) {
          *usedBytes = used_Bytes;
        }
        if (freeBytes) {
          *freeBytes = free_Bytes;
        }
        return true;
      }
    }
    default:
      break;
  }
  if (totalBytes) {
    *totalBytes = 0;
  }
  if (usedBytes) {
    *usedBytes = 0;
  }
  if (freeBytes) {
    *freeBytes = 0;
  }
  return false;
}

DIR *ESP3DGlobalFileSystem::opendir(const char *dirpath) {
  ESP3DFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      rewinddir(&_rootDir);
      return &_rootDir;
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd: {
      DIR *dirfd = sd.opendir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
      if (dirfd) {
        dirfd->dd_rsv = GLOBAL_SD_DIR_ID;
        return dirfd;
      }
    }
#endif  // ESP3D_SD_CARD_FEATURE
    break;
    case ESP3DFileSystemType::flash: {
      DIR *dirfd = flashFs.opendir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
      if (dirfd) {
        dirfd->dd_rsv = GLOBAL_FLASH_DIR_ID;
        return dirfd;
      }
    } break;
    default:
      break;
  }
  return NULL;
}
int ESP3DGlobalFileSystem::closedir(DIR *dirp) {
  if (dirp) {
    switch (dirp->dd_rsv) {
      case GLOBAL_ROOT_DIR_ID:
        return 0;
#if ESP3D_SD_CARD_FEATURE
      case GLOBAL_SD_DIR_ID:
        return sd.closedir(dirp);
#endif  // ESP3D_SD_CARD_FEATURE
      case GLOBAL_FLASH_DIR_ID:
        return flashFs.closedir(dirp);
      default:
        break;
    }
  }
  return -1;
}
int ESP3DGlobalFileSystem::stat(const char *filepath, struct stat *entry_stat) {
  esp3d_log("stat for %s", filepath);
  ESP3DFileSystemType fstype = getFSType(filepath);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      memset(entry_stat, 0, sizeof(struct stat));
      entry_stat->st_mode = S_IFDIR;
      time_t now;
      time(&now);
      entry_stat->st_mtime = now - (esp3d_hal::millis() / 1000);
      entry_stat->st_atime = entry_stat->st_mtime;
      entry_stat->st_ctime = entry_stat->st_mtime;
      return 0;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      esp3d_log("Is SD %s", filepath);
      return sd.stat(&filepath[strlen(ESP3D_SD_FS_HEADER) - 1], entry_stat);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.stat(&filepath[strlen(ESP3D_FLASH_FS_HEADER) - 1],
                          entry_stat);
    default:
      break;
  }
  return -1;
}

bool ESP3DGlobalFileSystem::exists(const char *path) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      if (strlen(path) == 0 && path[0] == '/') {
        return true;
      }
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.exists(&path[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.exists(&path[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool ESP3DGlobalFileSystem::remove(const char *path) {
  ESP3DFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.remove(&path[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.remove(&path[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool ESP3DGlobalFileSystem::mkdir(const char *dirpath) {
  ESP3DFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.mkdir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.mkdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool ESP3DGlobalFileSystem::rmdir(const char *dirpath) {
  ESP3DFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.rmdir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.rmdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool ESP3DGlobalFileSystem::rename(const char *oldpath, const char *newpath) {
  ESP3DFileSystemType fstypeold = getFSType(oldpath);
  ESP3DFileSystemType fstypenew = getFSType(newpath);
  if (fstypeold == fstypenew) {
    switch (fstypeold) {
      case ESP3DFileSystemType::root:
        break;
#if ESP3D_SD_CARD_FEATURE
      case ESP3DFileSystemType::sd:
        return sd.rename(&oldpath[strlen(ESP3D_SD_FS_HEADER) - 1],
                         &newpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
      case ESP3DFileSystemType::flash:
        return flashFs.rename(&oldpath[strlen(ESP3D_FLASH_FS_HEADER) - 1],
                              &newpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
      default:
        break;
    }
  }
  return false;
}

struct dirent *ESP3DGlobalFileSystem::readdir(DIR *dirp) {
  if (dirp) {
    switch (dirp->dd_rsv) {
      case GLOBAL_ROOT_DIR_ID:

        if (_rootEntry.d_ino < rootDirsHeadersSize) {
          strncpy(_rootEntry.d_name, &rootDirsHeaders[_rootEntry.d_ino][1],
                  strlen(rootDirsHeaders[_rootEntry.d_ino]) - 2);
          _rootEntry.d_name[strlen(rootDirsHeaders[_rootEntry.d_ino]) - 2] =
              '\0';
          _rootEntry.d_ino++;
          return &_rootEntry;
        }
        break;
#if ESP3D_SD_CARD_FEATURE
      case GLOBAL_SD_DIR_ID:
        return sd.readdir(dirp);
#endif  // ESP3D_SD_CARD_FEATURE
      case GLOBAL_FLASH_DIR_ID:
        return flashFs.readdir(dirp);
      default:
        break;
    }
  }
  return NULL;
}

void ESP3DGlobalFileSystem::rewinddir(DIR *dirp) {
  if (!dirp) {
    return;
  }
  switch (dirp->dd_rsv) {
    case GLOBAL_ROOT_DIR_ID:
      _rootEntry.d_ino = 0;
      _rootEntry.d_type = DT_DIR;
      strncpy(_rootEntry.d_name, &rootDirsHeaders[0][1],
              strlen(rootDirsHeaders[0]) - 2);
      _rootEntry.d_name[strlen(rootDirsHeaders[0]) - 2] = '\0';
      break;
#if ESP3D_SD_CARD_FEATURE
    case GLOBAL_SD_DIR_ID:
      sd.rewinddir(dirp);
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case GLOBAL_FLASH_DIR_ID:
      flashFs.rewinddir(dirp);
      break;
    default:
      break;
  }
}

FILE *ESP3DGlobalFileSystem::open(const char *filename, const char *mode) {
  ESP3DFileSystemType fstype = getFSType(filename);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      return sd.open(&filename[strlen(ESP3D_SD_FS_HEADER) - 1], mode);
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      return flashFs.open(&filename[strlen(ESP3D_FLASH_FS_HEADER) - 1], mode);
    default:
      break;
  }
  return NULL;
}

void ESP3DGlobalFileSystem::close(FILE *fd, const char *filename) {
  ESP3DFileSystemType fstype = getFSType(filename);
  switch (fstype) {
    case ESP3DFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::sd:
      sd.close(fd);
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DFileSystemType::flash:
      flashFs.close(fd);
      break;
    default:
      break;
  }
}
