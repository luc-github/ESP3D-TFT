
/*
  esp3d-settings.h -  settings esp3d functions class

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

typedef enum  {
    esp3d_version,
    last_one
} esp3d_setting_index_t;


typedef enum  {
    esp3d_byte, //byte
    esp3d_integer, //4 bytes
    esp3d_string, //string
    esp3d_ip, //4 bytes
} esp3d_setting_type_t;

typedef struct  {
    esp3d_setting_index_t index;
    esp3d_setting_type_t type;
    uint16_t size;
    const char* defaultval;
} Esp3DSetting_t;

class Esp3DSettings final
{
public:
    Esp3DSettings();
    ~Esp3DSettings();
    bool isValidSettingsNvs();
    uint8_t readByte(esp3d_setting_index_t index, bool * haserror = NULL);
    uint32_t readUint32(esp3d_setting_index_t index, bool * haserror = NULL);
    const char* readString(esp3d_setting_index_t index, char* out_str, size_t len, bool * haserror = NULL);
    bool writeByte (esp3d_setting_index_t index, const uint8_t value);
    bool writeUint32 (esp3d_setting_index_t index, const uint32_t value);
    bool writeString (esp3d_setting_index_t index, const char * byte_buffer);
    bool reset();

private:
    const Esp3DSetting_t * getSettingPtr(esp3d_setting_index_t index);
};

extern Esp3DSettings esp3dTFTsettings;