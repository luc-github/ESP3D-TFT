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

#include "sd_def.h"

#if defined(ESP3D_SD_IS_SPI) && ESP3D_SD_IS_SPI
#include "filesystem/esp3d_sd.h"
#include <string>
#include <cstring>
#include <string.h>
#include "esp3d_string.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp3d_log.h"
#include "esp3d-settings.h"

const char mount_point[] = "/sd";
sdmmc_card_t *card;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

void ESP3D_SD::unmount()
{
    if (!_started ) {
        esp3d_log_e("SDCard not init.");
        _state = ESP3D_SDCARD_UNKNOWN;
        return;
    }
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    _state = ESP3D_SDCARD_NOT_PRESENT;
    _mounted = false;
}

bool ESP3D_SD::mount()
{
    if (!_started) {
        esp3d_log_e("SDCard not init.");
        _state = ESP3D_SDCARD_UNKNOWN;
        return false;
    }
    if (_mounted) {
        unmount();
    }
    //set SPI speed

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)ESP3D_SD_CS_PIN;
    slot_config.host_id = (spi_host_device_t)host.slot;
    host.max_freq_khz = ESP3D_SD_FREQ / _spi_speed_divider;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = ESP3D_SD_FORMAT_IF_MOUNT_FAILED,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    esp3d_log("Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        _state = ESP3D_SDCARD_NOT_PRESENT;
        if (ret == ESP_FAIL) {
            esp3d_log_e("Failed to mount filesystem.");
        } else {
            esp3d_log_e("Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return false;
    }
    esp3d_log("Filesystem mounted");
    _mounted = true;
    _state = ESP3D_SDCARD_IDLE;
    return _mounted;
}

const char * ESP3D_SD::getFileSystemName()
{
    return "SD native";
}

bool ESP3D_SD::begin()
{
    _started =false;
    esp_err_t ret;
    esp3d_log("Initializing SD card");
    esp3d_log("Using SPI peripheral");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = (gpio_num_t)ESP3D_SD_MOSI_PIN,
        .miso_io_num = (gpio_num_t)ESP3D_SD_MISO_PIN,
        .sclk_io_num = (gpio_num_t)ESP3D_SD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = MAX_TRANSFER_SZ,
        .flags = 0,
        .intr_flags = 0,
    };
    _spi_speed_divider =
        ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        esp3d_log_e("Failed to initialize bus.");
        return false;
    }
    _started = true;
    return true;
}

uint ESP3D_SD::maxPathLength()
{
    return 255;
}

bool ESP3D_SD::getSpaceInfo(uint64_t * totalBytes,
                            uint64_t *usedBytes,
                            uint64_t *freeBytes, bool refreshStats)
{
    static uint64_t _totalBytes = 0;
    static uint64_t _usedBytes=0;
    static uint64_t _freeBytes=0;
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
        FATFS *fs;
        DWORD fre_clust;
        //we only have one SD card with one partiti0n so should be ok to use "0:"
        if(f_getfree("0:", &fre_clust, &fs) == FR_OK) {
            _totalBytes = (fs->n_fatent - 2) * fs->csize;
            _freeBytes = fre_clust * fs->csize;
            _totalBytes =  _totalBytes  * (fs->ssize);
            _freeBytes =   _freeBytes * (fs->ssize);
            _usedBytes = _totalBytes-_freeBytes;
        } else {
            esp3d_log_e("Failed to get total and free space");
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

const char * ESP3D_SD::getFullPath(const char * path)
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


#endif//ESP3D_SD_IS_SPI 