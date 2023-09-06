/*
  esp_flash_fatFs.cpp - ESP3D SD support class

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
#if ESP3D_FATFS_FEATURE
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include <cstring>
#include <string>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "filesystem/esp3d_flash.h"
#include "sdkconfig.h"

#define PARTITION_LABEL "flashfs"

wl_handle_t esp3d_fat_wl_handle = WL_INVALID_HANDLE;

void ESP3DFlash::unmount() {
  if (!_started) {
    esp3d_log_e("FatFs not init.");
    return;
  }
  esp_err_t err =
      esp_vfs_fat_spiflash_unmount_rw_wl(PARTITION_LABEL, esp3d_fat_wl_handle);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to unmount FATFS (%s)", esp_err_to_name(err));
  }
  _mounted = false;
}

bool ESP3DFlash::mount() {
  if (_mounted) {
    unmount();
  }
  const esp_vfs_fat_mount_config_t mount_config = {
      .format_if_mount_failed = true,
      .max_files = 4,
      .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
      .disk_status_check_enable = false};
  esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(
      mount_point(), PARTITION_LABEL, &mount_config, &esp3d_fat_wl_handle);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to mount FATFS (%s)", esp_err_to_name(err));
    return _mounted;
  }

  esp3d_log("Flash Filesystem FatFS mounted");
  _mounted = true;
#if ESP3D_TFT_LOG
  size_t totalBytes;
  size_t usedBytes;
  size_t freeBytes;
  if (getSpaceInfo(&totalBytes, &usedBytes, &freeBytes, true)) {
    esp3d_log("Total:%s", esp3d_string::formatBytes(totalBytes));
    esp3d_log("Used:%s", esp3d_string::formatBytes(usedBytes));
    esp3d_log("Free:%s", esp3d_string::formatBytes(freeBytes));
  }
#endif  // ESP3D_TFT_LOG

  return _mounted;
}

const char *ESP3DFlash::getFileSystemName() { return "FatFS"; }

bool ESP3DFlash::format() {
  if (_mounted) {
    unmount();
  }
  bool isFormated = false;
  esp_err_t err =
      esp_vfs_fat_spiflash_format_rw_wl(PARTITION_LABEL, mount_point());

  if (ESP_OK == err) {
    isFormated = true;
  } else {
    esp3d_log_e("Failed to format FATFS (%s)", esp_err_to_name(err));
  }
  mount();
  return (isFormated && _mounted);
}

bool ESP3DFlash::ESP3DFlash::begin() {
  _started = mount();
  getSpaceInfo();
  return _started;
}

uint ESP3DFlash::maxPathLength() { return CONFIG_LITTLEFS_OBJ_NAME_LEN; }

bool ESP3DFlash::getSpaceInfo(size_t *totalBytes, size_t *usedBytes,
                              size_t *freeBytes, bool refreshStats) {
  static size_t _totalBytes = 0;
  static size_t _usedBytes = 0;
  static size_t _freeBytes = 0;
  // esp3d_log("Try to get total and free space");
  // if not mounted reset values
  if (!_mounted) {
    esp3d_log_e("Failed to get total and free space because not mounted");
    _totalBytes = 0;
    _usedBytes = 0;
    _freeBytes = 0;
  }
  // no need to try if not  mounted
  if ((_totalBytes == 0 || refreshStats) && _mounted) {
    FATFS *fs;
    DWORD fre_clust;
    // we only have one SD card with one partiti0n so should be ok to use "0:"
    if (f_getfree("0:", &fre_clust, &fs) == FR_OK) {
      _totalBytes = (fs->n_fatent - 2) * fs->csize;
      _freeBytes = fre_clust * fs->csize;
      _totalBytes = _totalBytes * (fs->ssize);
      _freeBytes = _freeBytes * (fs->ssize);
      _usedBytes = _totalBytes - _freeBytes;
    } else {
      esp3d_log_e("Failed to get total and free space");
      _totalBytes = 0;
      _usedBytes = 0;
      _freeBytes = 0;
    }
  }
  // answer sizes according request
  if (totalBytes) {
    *totalBytes = _totalBytes;
  }
  if (usedBytes) {
    *usedBytes = _usedBytes;
  }
  if (freeBytes) {
    *freeBytes = _freeBytes;
  }
  // if total is 0 it is a failure
  return _totalBytes != 0;
}

DIR *ESP3DFlash::opendir(const char *dirpath) {
  std::string dir_path = mount_point();
  if (strlen(dirpath) != 0) {
    if (dirpath[0] != '/') {
      dir_path += "/";
    }
    dir_path += dirpath;
  }
  esp3d_log("openDir %s", dir_path.c_str());
  return ::opendir(dir_path.c_str());
}

int ESP3DFlash::closedir(DIR *dirp) { return ::closedir(dirp); }

int ESP3DFlash::stat(const char *filepath, struct stat *entry_stat) {
  std::string dir_path = mount_point();
  if (strlen(filepath) != 0) {
    if (filepath[0] != '/') {
      dir_path += "/";
    }
    dir_path += filepath;
  }
  // esp3d_log("Stat %s, %d", dir_path.c_str(), ::stat(dir_path.c_str(),
  // entry_stat));
  return ::stat(dir_path.c_str(), entry_stat);
}

bool ESP3DFlash::exists(const char *path) {
  struct stat entry_stat;
  if (stat(path, &entry_stat) == 0) {
    return true;
  } else {
    return false;
  }
}

bool ESP3DFlash::remove(const char *path) {
  std::string file_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      file_path += "/";
    }
    file_path += path;
  }
  return !unlink(file_path.c_str());
}

bool ESP3DFlash::mkdir(const char *path) {
  std::string dir_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      dir_path += "/";
    }
    dir_path += path;
  }
  return !::mkdir(dir_path.c_str(), 0777);
}

bool ESP3DFlash::rmdir(const char *path) {
  std::string dir_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      dir_path += "/";
    }
    dir_path += path;
  }
  return !::rmdir(dir_path.c_str());
}
bool ESP3DFlash::rename(const char *oldpath, const char *newpath) {
  std::string old_path = mount_point();
  std::string new_path = mount_point();
  if (strlen(oldpath) != 0) {
    if (oldpath[0] != '/') {
      old_path += "/";
    }
    old_path += oldpath;
  }
  if (strlen(newpath) != 0) {
    if (newpath[0] != '/') {
      new_path += "/";
    }
    new_path += newpath;
  }
  struct stat st;
  if (::stat(new_path.c_str(), &st) == 0) {
    ::unlink(new_path.c_str());
  }
  return !::rename(old_path.c_str(), new_path.c_str());
}

FILE *ESP3DFlash::open(const char *filename, const char *mode) {
  std::string file_path = mount_point();
  if (strlen(filename) != 0) {
    if (filename[0] != '/') {
      file_path += "/";
    }
    file_path += filename;
  }
  return fopen(file_path.c_str(), mode);
}

struct dirent *ESP3DFlash::readdir(DIR *dir) { return ::readdir(dir); }

void ESP3DFlash::rewinddir(DIR *dir) { ::rewinddir(dir); }

void ESP3DFlash::close(FILE *fd) {
  fclose(fd);
  fd = nullptr;
}

#endif  // ESP3D_FATFS_FEATURE