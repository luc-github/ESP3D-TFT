/*
  esp_sd_sdio.cpp - ESP3D SD support class

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

#if defined(ESP3D_SD_IS_SDIO) && ESP3D_SD_IS_SDIO
#include <string.h>
#include <sys/unistd.h>

#include <cstring>
#include <string>

#include "driver/sdmmc_host.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "filesystem/esp3d_sd.h"
#include "sdkconfig.h"
#include "sdmmc_cmd.h"

sdmmc_card_t *card;

void ESP3DSd::unmount() {
  if (!_started) {
    esp3d_log_e("SDCard not init.");
    _state = ESP3DSdState::unknown;
    return;
  }
  esp_vfs_fat_sdcard_unmount(mount_point(), card);
  _state = ESP3DSdState::not_present;
  _mounted = false;
}

bool ESP3DSd::mount() {
  if (!_started) {
    esp3d_log_e("SDCard not init.");
    _state = ESP3DSdState::unknown;
    return false;
  }
  if (_mounted) {
    unmount();
  }
  esp3d_log("Initializing SD card");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.max_freq_khz = ESP3D_SD_FREQ;
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = ESP3D_SDIO_BIT_WIDTH;
#if SOC_SDMMC_USE_GPIO_MATRIX
  slot_config.clk = (gpio_num_t)ESP3D_SDIO_CLK_PIN;
  slot_config.cmd = (gpio_num_t)ESP3D_SDIO_CMD_PIN;
  slot_config.d0 = (gpio_num_t)ESP3D_SDIO_D0_PIN;
  slot_config.d1 = (gpio_num_t)ESP3D_SDIO_D1_PIN;
  slot_config.d2 = (gpio_num_t)ESP3D_SDIO_D2_PIN;
  slot_config.d3 = (gpio_num_t)ESP3D_SDIO_D3_PIN;

#endif  // SOC_SDMMC_USE_GPIO_MATRIX
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  slot_config.cd = (gpio_num_t)ESP3D_SD_DETECT_PIN;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      /** New IDF 5.0, Try to enable if you need to handle situations when SD
       * cards are not unmounted properly before physical removal or you are
       * experiencing issues with SD cards.*/
      .disk_status_check_enable = false};
  esp3d_log("Mounting filesystem");
  esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point(), &host, &slot_config,
                                          &mount_config, &card);

  if (ret != ESP_OK) {
    _state = ESP3DSdState::not_present;
    if (ret == ESP_FAIL) {
      esp3d_log_e("Failed to mount filesystem.");
    } else {
      esp3d_log_e("Failed to initialize the card (%s). ", esp_err_to_name(ret));
    }
    return false;
  }
  esp3d_log("Filesystem mounted");
  _mounted = true;
  _state = ESP3DSdState::idle;
  return _mounted;
}

const char *ESP3DSd::getFileSystemName() { return "SDFat native"; }

bool ESP3DSd::begin() {
  _started = true;
  return true;
}

uint ESP3DSd::maxPathLength() { return CONFIG_FATFS_MAX_LFN; }

bool ESP3DSd::getSpaceInfo(uint64_t *totalBytes, uint64_t *usedBytes,
                           uint64_t *freeBytes, bool refreshStats) {
  static uint64_t _totalBytes = 0;
  static uint64_t _usedBytes = 0;
  static uint64_t _freeBytes = 0;
  esp3d_log("Try to get total and free space");
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
DIR *ESP3DSd::opendir(const char *dirpath) {
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

int ESP3DSd::closedir(DIR *dirp) { return ::closedir(dirp); }

int ESP3DSd::stat(const char *filepath, struct stat *entry_stat) {
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

bool ESP3DSd::exists(const char *path) {
  struct stat entry_stat;
  if (stat(path, &entry_stat) == 0) {
    return true;
  } else {
    return false;
  }
}

bool ESP3DSd::remove(const char *path) {
  std::string file_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      file_path += "/";
    }
    file_path += path;
  }
  return !::unlink(file_path.c_str());
}

bool ESP3DSd::mkdir(const char *path) {
  std::string dir_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      dir_path += "/";
    }
    dir_path += path;
  }
  return !::mkdir(dir_path.c_str(), 0777);
}

bool ESP3DSd::rmdir(const char *path) {
  std::string dir_path = mount_point();
  if (strlen(path) != 0) {
    if (path[0] != '/') {
      dir_path += "/";
    }
    dir_path += path;
  }
  return !::rmdir(dir_path.c_str());
}
bool ESP3DSd::rename(const char *oldpath, const char *newpath) {
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

FILE *ESP3DSd::open(const char *filename, const char *mode) {
  std::string file_path = mount_point();
  if (strlen(filename) != 0) {
    if (filename[0] != '/') {
      file_path += "/";
    }
    file_path += filename;
  }
  return fopen(file_path.c_str(), mode);
}

struct dirent *ESP3DSd::readdir(DIR *dir) { return ::readdir(dir); }

void ESP3DSd::rewinddir(DIR *dir) { ::rewinddir(dir); }

void ESP3DSd::close(FILE *fd) { fclose(fd); }

#endif  // ESP3D_SD_IS_SDIO