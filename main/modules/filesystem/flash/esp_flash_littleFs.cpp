/*
  esp_sd_spi.cpp - ESP3D SD support class

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


#include "filesystem/esp3d_flash.h"
#include <string>
#include <cstring>
#include <string.h>
#include "esp3d_string.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_flash.h"
#include "esp_littlefs.h"
#include "esp3d_log.h"

const char mount_point[] = "/fs";
#define PARTITION_LABEL "flashfs"

void ESP3D_FLASH::unmount()
{
    if (!_started ) {
        esp3d_log_e("LittleFs not init.");
        return;
    }
    esp_vfs_littlefs_unregister(PARTITION_LABEL);
    _mounted = false;
}

bool ESP3D_FLASH::mount()
{
    if (_mounted) {
        unmount();
    }
    esp_vfs_littlefs_conf_t conf = {
        .base_path = mount_point,
        .partition_label = PARTITION_LABEL,
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            esp3d_log_e("Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            esp3d_log_e("Failed to find LittleFS partition %s", PARTITION_LABEL);
        } else {
            esp3d_log_e("Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return false;
    } else {
        esp3d_log("Flash Filesystem mounted");
        _mounted = true;
#if ESP3D_TFT_LOG
        size_t totalBytes;
        size_t usedBytes;
        size_t freeBytes;
        if (getSpaceInfo(&totalBytes,
                         &usedBytes,
                         &freeBytes, true)) {
            esp3d_log("Total:%s", formatBytes(totalBytes));
            esp3d_log("Used:%s", formatBytes(usedBytes));
            esp3d_log("Free:%s", formatBytes(freeBytes));
        }
#endif //ESP3D_TFT_LOG
    }
    return _mounted;
}

const char * ESP3D_FLASH::getFileSystemName()
{
    return "LittleFS";
}

bool ESP3D_FLASH::begin()
{
    _started = mount();
    return _started;
}

uint ESP3D_FLASH::maxPathLength()
{
    return 255;
}

bool ESP3D_FLASH::getSpaceInfo(size_t * totalBytes,
                               size_t *usedBytes,
                               size_t *freeBytes, bool refreshStats)
{
    static size_t _totalBytes = 0;
    static size_t _usedBytes=0;
    static size_t _freeBytes=0;
    esp3d_log("Try to get total and free space");
    //if not mounted reset values
    if (!_mounted) {
        esp3d_log_e("Failed to get total and free space because not mounted");
        _totalBytes = 0;
        _usedBytes=0;
        _freeBytes=0;
    }
    //no need to try if not  mounted
    if ((_totalBytes == 0 || refreshStats) && _mounted) {
        esp_err_t  res = esp_littlefs_info(PARTITION_LABEL, &_totalBytes, &_usedBytes);
        if (ESP_OK== res ) {
            _freeBytes = _totalBytes - _usedBytes;
            esp3d_log("Total:%d, Used:%d, Free:%d", (_totalBytes), (_usedBytes),(_freeBytes));
        } else {
            esp3d_log_e("Failed to get total and free space because %s",esp_err_to_name(res));
            _totalBytes = 0;
            _usedBytes=0;
            _freeBytes=0;
        }
    }
    //answer sizes according request
    if (totalBytes) {
        *totalBytes=_totalBytes;
    }
    if (usedBytes) {
        *usedBytes=_usedBytes;
    }
    if (freeBytes) {
        *freeBytes=_freeBytes;
    }
    //if total is 0 it is a failure
    return _totalBytes!=0;
}

const char * ESP3D_FLASH::getFullPath(const char * path)
{
    static std::string fullpath = mount_point;
    std::string tpath = str_trim(path);
    if (path && tpath.c_str()[0] != '/') {
        fullpath += "/";
    }
    fullpath += tpath;
    if (fullpath.c_str()[fullpath.size()-1]!='/') {
        fullpath += "/";
    }
    return fullpath.c_str();
}
