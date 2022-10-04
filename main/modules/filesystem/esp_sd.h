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

#define ESP_SD_FS_HEADER "/sd/"

class ESP_SD final
{
public:
    ESP_SD();
    /*char * formatBytes (uint64_t bytes);*/
    bool begin();
    /*bool  accessFS(uint8_t FS = FS_SD);
    void  releaseFS(uint8_t FS = FS_SD);
    uint8_t getFSType(const char * path=nullptr);
    void handle();
    void end();
    uint8_t getState(bool refresh=false);
    uint8_t setState(uint8_t state);
    void refreshStats(bool force = false);
    uint64_t totalBytes(bool refresh = false);
    uint64_t usedBytes(bool refresh = false);
    uint64_t freeBytes(bool refresh = false);
    uint maxPathLength();
    const char * FilesystemName();
    bool format(ESP3DOutput * output = nullptr);
    FILE open(const char* path, uint8_t mode = ESP_FILE_READ);
    bool exists(const char* path);
    bool remove(const char *path);
    bool mkdir(const char *path);
    bool rmdir(const char *path);
    bool rename(const char *oldpath, const char *newpath);
    void closeAll();
    uint8_t getSPISpeedDivider()
    {
        return _spi_speed_divider;
    }
    bool setSPISpeedDivider(uint8_t speeddivider);*/
private:
    FILE _currentFile;
    bool _started;
    uint8_t _state;
    uint8_t _spi_speed_divider;
    bool _sizechanged;
};

extern ESP_SD sd;