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

#if defined(ESP_SD_IS_SPI) && ESP_SD_IS_SPI
#include "filesystem/esp3d_sd.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

#define LOG_TAG "SD_SPI"

const char mount_point[] = "/sdcard";
sdmmc_card_t *card;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

void ESP_SD::unmount()
{
    if (!_started ) {
        ESP_LOGE(LOG_TAG, "SDCard not init.");
        return;
    }
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    _mounted = false;
}

bool ESP_SD::mount()
{
    if (!_started) {
        ESP_LOGE(LOG_TAG, "SDCard not init.");
        return false;
    }
    if (_mounted) {
        unmount();
    }
    //set SPI speed

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)ESP_SD_CS_PIN;
    slot_config.host_id = (spi_host_device_t)host.slot;
    host.max_freq_khz = ESP_SD_FREQ / _spi_speed_divider;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = ESP_SD_FORMAT_IF_MOUNT_FAILED,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    ESP_LOGI(LOG_TAG, "Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(LOG_TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(LOG_TAG, "Failed to initialize the card (%s). ", esp_err_to_name(ret));
        }
        return false;
    }
    ESP_LOGI(LOG_TAG, "Filesystem mounted");
    _mounted = true;
    return _mounted;
}

const char * ESP_SD::getFileSystemName()
{
    return "SD native";
}

bool ESP_SD::begin()
{
    _started =false;
    esp_err_t ret;
    ESP_LOGI(LOG_TAG, "Initializing SD card");
    ESP_LOGI(LOG_TAG, "Using SPI peripheral");
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = (gpio_num_t)ESP_SD_MOSI_PIN,
        .miso_io_num = (gpio_num_t)ESP_SD_MISO_PIN,
        .sclk_io_num = (gpio_num_t)ESP_SD_CLK_PIN,
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
    ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Failed to initialize bus.");
        return false;
    }
    _started = true;
    return true;
}

uint ESP_SD::maxPathLength()
{
    return 255;
}

bool ESP_SD::getSdInfo(uint64_t * totalBytes,
                       uint64_t *usedBytes,
                       uint64_t *freeBytes, bool refreshStats)
{
    static uint64_t _totalBytes = 0;
    static uint64_t _usedBytes=0;
    static uint64_t _freeBytes=0;
    ESP_LOGI(LOG_TAG, "Try to get total and free space");
    //if not mounted reset values
    if (!_mounted) {
        ESP_LOGE(LOG_TAG, "Failed to get total and free space because not mounted");
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
            _totalBytes = 1024 * (_totalBytes / 2);
            _freeBytes = 1024 * (_freeBytes / 2);
            _usedBytes = _totalBytes-_freeBytes;
        } else {
            ESP_LOGE(LOG_TAG, "Failed to get total and free space");
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

#endif//ESP_SD_IS_SPI 