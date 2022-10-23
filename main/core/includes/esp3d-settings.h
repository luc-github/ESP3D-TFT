
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
#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    esp3d_version,
    esp3d_baud_rate,
    last_one
} esp3d_setting_index_t;


typedef enum  {
    esp3d_byte, //byte
    esp3d_integer, //4 bytes
    esp3d_string, //string
    esp3d_ip, //4 bytes
    esp3d_float, //4 bytes
    esp3d_mask, //x bytes
    esp3d_bitsfield, //x bytes
} esp3d_setting_type_t;

//to be implemented :
//use regex could also be implemented
//for options list may be use a map list int is value and string is display
//std::map<int, std::string> options { {0, "No Network"}, {1, "AP"}, {2, "STA"},}

typedef struct {
    int64_t max;
    int64_t min;
    bool canbeempty;
    void * options;
}  esp3d_setting_boundaries_t;

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
    const char* readIPString(esp3d_setting_index_t index, bool * haserror = NULL);
    const char* readString(esp3d_setting_index_t index, char* out_str, size_t len, bool * haserror = NULL);
    bool writeByte (esp3d_setting_index_t index, const uint8_t value);
    bool writeUint32 (esp3d_setting_index_t index, const uint32_t value);
    bool writeIPString (esp3d_setting_index_t index, const char * byte_buffer);
    bool writeString (esp3d_setting_index_t index, const char * byte_buffer);
    bool reset();
    bool isValidStringSetting(const char* value, esp3d_setting_index_t settingElement);
    bool isValidIntegerSetting(uint32_t value, esp3d_setting_index_t settingElement);
    bool isValidByteSetting(uint8_t value, esp3d_setting_index_t settingElement);
    uint32_t getDefaultIntegerSetting(esp3d_setting_index_t settingElement);
    const char * getDefaultStringSetting(esp3d_setting_index_t settingElement);
    uint8_t getDefaultByteSetting(esp3d_setting_index_t settingElement);
    const Esp3DSetting_t * getSettingPtr(esp3d_setting_index_t index);
private:
    const char *IPUInt32toString(uint32_t ip_int);
    uint32_t StringtoIPUInt32(const char *s);

};

extern Esp3DSettings esp3dTFTsettings;

#ifdef __cplusplus
} // extern "C"
#endif