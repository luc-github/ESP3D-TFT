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

#include <cstring>
#include <string>

#include "esp3d_flash.h"
#include "esp3d_log.h"
#include "esp3d_string.h"

#if ESP3D_SD_CARD_FEATURE
#include "esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

Esp3dGlobalFileSystem globalFs;
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
const Esp3dFileSystemType rootDirsFSType[] = {Esp3dFileSystemType::flash
#if ESP3D_SD_CARD_FEATURE
                                              ,
                                              Esp3dFileSystemType::sd
#endif  // ESP3D_SD_CARD_FEATURE
};

Esp3dGlobalFileSystem::Esp3dGlobalFileSystem() {
  _rootDir.dd_vfs_idx = (uint16_t)-1;
  _rootDir.dd_rsv = GLOBAL_ROOT_DIR_ID;
  rewinddir(&_rootDir);
}

const char *Esp3dGlobalFileSystem::mount_point(Esp3dFileSystemType fstype) {
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return "/";
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.mount_point();
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.mount_point();
    default:
      break;
  }
  return "/";
}

Esp3dFileSystemType Esp3dGlobalFileSystem::getFSType(const char *path) {
  if (!path || strlen(path) == 0) {
    return Esp3dFileSystemType::unknown;
  }
  if (strlen(path) == 1 && path[0] == '/') {
    return Esp3dFileSystemType::root;
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
  return Esp3dFileSystemType::unknown;
}

bool Esp3dGlobalFileSystem::accessFS(const char *path) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return true;
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.accessFS();
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.accessFS();
    default:
      break;
  }
  return false;
}

void Esp3dGlobalFileSystem::releaseFS(const char *path) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      sd.releaseFS();
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      flashFs.releaseFS();
      break;
    default:
      break;
  }
}

bool Esp3dGlobalFileSystem::begin() { return true; }

const char *Esp3dGlobalFileSystem::getFileSystemName(char *path) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return "Global";
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.getFileSystemName();
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.getFileSystemName();
    default:
      break;
  }
  return "Unknown";
}
uint Esp3dGlobalFileSystem::maxPathLength(Esp3dFileSystemType fstype) {
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return 0;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.maxPathLength();
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.maxPathLength();
    default:
      break;
  }
  return false;
}
bool Esp3dGlobalFileSystem::getSpaceInfo(uint64_t *totalBytes,
                                         uint64_t *usedBytes,
                                         uint64_t *freeBytes, const char *path,
                                         bool refreshStats) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return false;
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.getSpaceInfo(totalBytes, usedBytes, freeBytes, refreshStats);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash: {
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

DIR *Esp3dGlobalFileSystem::opendir(const char *dirpath) {
  Esp3dFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      rewinddir(&_rootDir);
      return &_rootDir;
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd: {
      DIR *dirfd = sd.opendir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
      if (dirfd) {
        dirfd->dd_rsv = GLOBAL_SD_DIR_ID;
        return dirfd;
      }
    }
#endif  // ESP3D_SD_CARD_FEATURE
    break;
    case Esp3dFileSystemType::flash: {
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
int Esp3dGlobalFileSystem::closedir(DIR *dirp) {
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
int Esp3dGlobalFileSystem::stat(const char *filepath, struct stat *entry_stat) {
  esp3d_log("stat for %s", filepath);
  Esp3dFileSystemType fstype = getFSType(filepath);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      return -1;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      esp3d_log("Is SD %s", filepath);
      return sd.stat(&filepath[strlen(ESP3D_SD_FS_HEADER) - 1], entry_stat);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.stat(&filepath[strlen(ESP3D_FLASH_FS_HEADER) - 1],
                          entry_stat);
    default:
      break;
  }
  return -1;
}

bool Esp3dGlobalFileSystem::exists(const char *path) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      if (strlen(path) == 0 && path[0] == '/') {
        return true;
      }
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.exists(&path[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.exists(&path[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool Esp3dGlobalFileSystem::remove(const char *path) {
  Esp3dFileSystemType fstype = getFSType(path);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.remove(&path[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.remove(&path[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool Esp3dGlobalFileSystem::mkdir(const char *dirpath) {
  Esp3dFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.mkdir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.mkdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool Esp3dGlobalFileSystem::rmdir(const char *dirpath) {
  Esp3dFileSystemType fstype = getFSType(dirpath);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.rmdir(&dirpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.rmdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
    default:
      break;
  }
  return false;
}

bool Esp3dGlobalFileSystem::rename(const char *oldpath, const char *newpath) {
  Esp3dFileSystemType fstypeold = getFSType(oldpath);
  Esp3dFileSystemType fstypenew = getFSType(newpath);
  if (fstypeold == fstypenew) {
    switch (fstypeold) {
      case Esp3dFileSystemType::root:
        break;
#if ESP3D_SD_CARD_FEATURE
      case Esp3dFileSystemType::sd:
        return sd.rename(&oldpath[strlen(ESP3D_SD_FS_HEADER) - 1],
                         &newpath[strlen(ESP3D_SD_FS_HEADER) - 1]);
#endif  // ESP3D_SD_CARD_FEATURE
      case Esp3dFileSystemType::flash:
        return flashFs.rename(&oldpath[strlen(ESP3D_FLASH_FS_HEADER) - 1],
                              &newpath[strlen(ESP3D_FLASH_FS_HEADER) - 1]);
      default:
        break;
    }
  }
  return false;
}

struct dirent *Esp3dGlobalFileSystem::readdir(DIR *dirp) {
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

void Esp3dGlobalFileSystem::rewinddir(DIR *dirp) {
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

FILE *Esp3dGlobalFileSystem::open(const char *filename, const char *mode) {
  Esp3dFileSystemType fstype = getFSType(filename);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      return sd.open(&filename[strlen(ESP3D_SD_FS_HEADER) - 1], mode);
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      return flashFs.open(&filename[strlen(ESP3D_FLASH_FS_HEADER) - 1], mode);
    default:
      break;
  }
  return NULL;
}

void Esp3dGlobalFileSystem::close(FILE *fd, const char *filename) {
  Esp3dFileSystemType fstype = getFSType(filename);
  switch (fstype) {
    case Esp3dFileSystemType::root:
      break;
#if ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::sd:
      sd.close(fd);
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case Esp3dFileSystemType::flash:
      flashFs.close(fd);
      break;
    default:
      break;
  }
}
