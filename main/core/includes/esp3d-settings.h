
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

#define SIZE_OF_SETTING_VERSION 25
#define SIZE_OF_SETTING_SSID_ID 32
#define SIZE_OF_SETTING_SSID_PWD 64
#define SIZE_OF_SETTING_HOSTNAME 32
#define HIDDEN_SETTING_VALUE "********"

typedef enum  {
    esp3d_version,
    esp3d_baud_rate,
    esp3d_spi_divider,
    esp3d_radio_boot_mode,
    esp3d_radio_mode,
    esp3d_fallback_mode,
    esp3d_sta_ssid,
    esp3d_sta_password,
    esp3d_sta_ip_mode,
    esp3d_sta_ip_static,
    esp3d_sta_mask_static,
    esp3d_sta_gw_static,
    esp3d_sta_dns_static,
    esp3d_ap_ssid,
    esp3d_ap_password,
    esp3d_ap_ip_static,
    esp3d_ap_channel,
    esp3d_hostname,
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
    esp3d_unknow
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
    bool isValidIPStringSetting(const char* value, esp3d_setting_index_t settingElement);
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