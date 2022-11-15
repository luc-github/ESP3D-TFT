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
#include <string>
#include <cstring>
#include <string.h>
#include "esp3d_string.h"
#include "esp3d_log.h"
#include "esp3d_globalfs.h"
#include "esp3d_flash.h"
#include "esp3d_sd.h"

ESP3D_GLOBALFS globalFs;
#define GLOBAL_ROOT_DIR_ID 8888
#define GLOBAL_FLASH_DIR_ID 1111
#define GLOBAL_SD_DIR_ID 2222

const char * rootDirsHeaders[]= {ESP3D_FLASH_FS_HEADER,ESP3D_SD_FS_HEADER};
uint8_t rootDirsHeadersSize = sizeof(rootDirsHeaders)/sizeof(char*);
const uint rootDirsFSType[]= {FS_FLASH,FS_SD};

ESP3D_GLOBALFS::ESP3D_GLOBALFS()
{
    _rootDir.dd_vfs_idx = (uint16_t)-1;
    _rootDir.dd_rsv = GLOBAL_ROOT_DIR_ID;
    rewinddir(&_rootDir);
}

const char * ESP3D_GLOBALFS::mount_point(esp3d_fs_types fstype)
{
    switch(fstype) {
    case FS_ROOT:
        return "/";
    case FS_SD:
        return sd.mount_point();
    case FS_FLASH:
        return flashFs.mount_point();
    default:
        break;
    }
    return "/";
}

esp3d_fs_types ESP3D_GLOBALFS::getFSType(const char * path)
{
    if (!path || strlen(path)==0) {
        return FS_UNKNOWN;
    }
    if (strlen(path)==1 && path[0]=='/') {
        return FS_ROOT;
    }
    std::string fullPath;
    if (path[0]!='/') {
        fullPath= "/";
        fullPath+=path;
    } else {
        fullPath=path;
    }
    fullPath+="/"; //to handle /sd and /fs

    uint sizePath = fullPath.length();
    for (uint j = 0; j< rootDirsHeadersSize; j++) {
        uint sizeHeader = strlen(rootDirsHeaders[j]);
        for (uint i = 0; i < sizeHeader && i<sizePath; i++) {
            if (fullPath[i]!=rootDirsHeaders[j][i]) {
                break;
            } else {
                if (i==sizeHeader-1) {
                    return (esp3d_fs_types)rootDirsFSType[j];
                }
            }
        }
    }
    return FS_UNKNOWN;
}

bool  ESP3D_GLOBALFS::accessFS(const char * path)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        return true;
        break;
    case FS_SD:
        return sd.accessFS();
    case FS_FLASH:
        return flashFs.accessFS();
    default:
        break;
    }
    return false;
}

void  ESP3D_GLOBALFS::releaseFS(const char * path)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        sd.releaseFS();
        break;
    case FS_FLASH:
        flashFs.releaseFS();
        break;
    default:
        break;
    }
}

bool ESP3D_GLOBALFS::begin()
{
    return true;
}

const char * ESP3D_GLOBALFS::getFileSystemName(char * path)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        return "Global";
        break;
    case FS_SD:
        return sd.getFileSystemName();
    case FS_FLASH:
        return flashFs.getFileSystemName();
    default:
        break;
    }
    return "Unknown";
}
uint ESP3D_GLOBALFS::maxPathLength(esp3d_fs_types fstype)
{
    switch(fstype) {
    case FS_ROOT:
        return 0;
    case FS_SD:
        return sd.maxPathLength();
    case FS_FLASH:
        return flashFs.maxPathLength();
    default:
        break;
    }
    return false;
}
bool ESP3D_GLOBALFS::getSpaceInfo(uint64_t * totalBytes,
                                  uint64_t * usedBytes,
                                  uint64_t * freeBytes,
                                  const char *path,
                                  bool refreshStats)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        return false;
        break;
    case FS_SD:
        return sd.getSpaceInfo(totalBytes,
                               usedBytes,
                               freeBytes,
                               refreshStats);
    case FS_FLASH: {
        size_t total_Bytes;
        size_t used_Bytes;
        size_t free_Bytes;
        bool res =  flashFs.getSpaceInfo(totalBytes?&total_Bytes:NULL,
                                         usedBytes?&used_Bytes:NULL,
                                         freeBytes?&free_Bytes:NULL,
                                         refreshStats);
        if (res) {
            if (totalBytes) {
                *totalBytes=total_Bytes;
            }
            if (usedBytes) {
                *usedBytes=used_Bytes;
            }
            if (freeBytes) {
                *freeBytes=free_Bytes;
            }
            return true;
        }
    }
    default:
        break;
    }
    if (totalBytes) {
        *totalBytes=0;
    }
    if (usedBytes) {
        *usedBytes=0;
    }
    if (freeBytes) {
        *freeBytes=0;
    }
    return false;
}

DIR * ESP3D_GLOBALFS::opendir(const char * dirpath)
{
    esp3d_fs_types fstype = getFSType(dirpath);
    switch(fstype) {
    case FS_ROOT:
        rewinddir(&_rootDir);
        return &_rootDir;
        break;
    case FS_SD: {
        DIR * dirfd = sd.opendir(&dirpath[strlen(ESP3D_SD_FS_HEADER)-1]);
        if (dirfd) {
            dirfd->dd_rsv = GLOBAL_SD_DIR_ID;
            return dirfd;
        }
    }
    break;
    case FS_FLASH: {
        DIR * dirfd = flashFs.opendir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER)-1]);
        if (dirfd) {
            dirfd->dd_rsv = GLOBAL_FLASH_DIR_ID;
            return dirfd;
        }
    }
    break;
    default:
        break;
    }
    return NULL;
}
int ESP3D_GLOBALFS::closedir(DIR *dirp)
{
    if (dirp) {
        switch(dirp->dd_rsv) {
        case GLOBAL_ROOT_DIR_ID:
            return 0;
        case GLOBAL_SD_DIR_ID:
            return sd.closedir(dirp);
        case GLOBAL_FLASH_DIR_ID:
            return flashFs.closedir(dirp);
        default:
            break;
        }
    }
    return -1;
}
int ESP3D_GLOBALFS::stat(const char * filepath,  struct  stat * entry_stat)
{
    esp3d_log("stat for %s", filepath);
    esp3d_fs_types fstype = getFSType(filepath);
    switch(fstype) {
    case FS_ROOT:
        return -1;
    case FS_SD:
        esp3d_log("Is SD %s", filepath);
        return sd.stat(&filepath[strlen(ESP3D_SD_FS_HEADER)-1],entry_stat);
    case FS_FLASH:
        return flashFs.stat(&filepath[strlen(ESP3D_FLASH_FS_HEADER)-1],entry_stat);
    default:
        break;
    }
    return -1;
}

bool ESP3D_GLOBALFS::exists(const char* path)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        if (strlen(path)==0 && path[0]=='/') {
            return true;
        }
        break;
    case FS_SD:
        return sd.exists(&path[strlen(ESP3D_SD_FS_HEADER)-1]);
    case FS_FLASH:
        return flashFs.exists(&path[strlen(ESP3D_FLASH_FS_HEADER)-1]);
    default:
        break;
    }
    return false;
}

bool ESP3D_GLOBALFS::remove(const char *path)
{
    esp3d_fs_types fstype = getFSType(path);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        return sd.remove(&path[strlen(ESP3D_SD_FS_HEADER)-1]);
    case FS_FLASH:
        return flashFs.remove(&path[strlen(ESP3D_FLASH_FS_HEADER)-1]);
    default:
        break;
    }
    return false;
}

bool ESP3D_GLOBALFS::mkdir(const char *dirpath)
{
    esp3d_fs_types fstype = getFSType(dirpath);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        return sd.mkdir(&dirpath[strlen(ESP3D_SD_FS_HEADER)-1]);
    case FS_FLASH:
        return flashFs.mkdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER)-1]);
    default:
        break;
    }
    return false;
}

bool ESP3D_GLOBALFS::rmdir(const char *dirpath)
{
    esp3d_fs_types fstype = getFSType(dirpath);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        return sd.rmdir(&dirpath[strlen(ESP3D_SD_FS_HEADER)-1]);
    case FS_FLASH:
        return flashFs.rmdir(&dirpath[strlen(ESP3D_FLASH_FS_HEADER)-1]);
    default:
        break;
    }
    return false;
}

bool ESP3D_GLOBALFS::rename(const char *oldpath, const char *newpath)
{
    esp3d_fs_types fstypeold = getFSType(oldpath);
    esp3d_fs_types fstypenew = getFSType(newpath);
    if (fstypeold == fstypenew) {
        switch(fstypeold) {
        case FS_ROOT:
            break;
        case FS_SD:
            return sd.rename(&oldpath[strlen(ESP3D_SD_FS_HEADER)-1],&newpath[strlen(ESP3D_SD_FS_HEADER)-1]);
        case FS_FLASH:
            return flashFs.rename(&oldpath[strlen(ESP3D_FLASH_FS_HEADER)-1],&newpath[strlen(ESP3D_SD_FS_HEADER)-1]);
        default:
            break;
        }
    }
    return false;
}

struct dirent *  ESP3D_GLOBALFS::readdir(DIR *dirp)
{
    if (dirp) {
        switch(dirp->dd_rsv) {
        case GLOBAL_ROOT_DIR_ID:

            if ( _rootEntry.d_ino<rootDirsHeadersSize) {
                strncpy(_rootEntry.d_name,&rootDirsHeaders[_rootEntry.d_ino][1], strlen(rootDirsHeaders[_rootEntry.d_ino])-2);
                _rootEntry.d_name[strlen(rootDirsHeaders[_rootEntry.d_ino])-2]='\0';
                _rootEntry.d_ino++;
                return &_rootEntry;
            }
            break;
        case GLOBAL_SD_DIR_ID:
            return sd.readdir(dirp);
        case GLOBAL_FLASH_DIR_ID:
            return flashFs.readdir(dirp);
        default:
            break;
        }
    }
    return NULL;
}

void ESP3D_GLOBALFS::rewinddir(DIR * dirp)
{
    if (!dirp) {
        return ;
    }
    switch(dirp->dd_rsv) {
    case GLOBAL_ROOT_DIR_ID:
        _rootEntry.d_ino=0;
        _rootEntry.d_type = DT_DIR;
        strncpy(_rootEntry.d_name,&rootDirsHeaders[0][1], strlen(rootDirsHeaders[0])-2);
        _rootEntry.d_name[strlen(rootDirsHeaders[0])-2]='\0';
        break;
    case GLOBAL_SD_DIR_ID:
        sd.rewinddir(dirp);
        break;
    case GLOBAL_FLASH_DIR_ID:
        flashFs.rewinddir(dirp);
        break;
    default:
        break;
    }
}

FILE * ESP3D_GLOBALFS::open ( const char * filename, const char * mode )
{
    esp3d_fs_types fstype = getFSType(filename);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        return sd.open (&filename[strlen(ESP3D_SD_FS_HEADER)-1],mode );
    case FS_FLASH:
        return flashFs.open (&filename[strlen(ESP3D_SD_FS_HEADER)-1],mode );
    default:
        break;
    }
    return NULL;
}

void ESP3D_GLOBALFS::close(FILE * fd, const char * filename)
{
    esp3d_fs_types fstype = getFSType(filename);
    switch(fstype) {
    case FS_ROOT:
        break;
    case FS_SD:
        sd.close(fd);
        break;
    case FS_FLASH:
        flashFs.close(fd);
        break;
    default:
        break;
    }

}

