
/*
  esp3d_settings.h -  settings esp3d functions class

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
#define SIZE_OF_SETTING_NOFIFICATION_T1 64
#define SIZE_OF_SETTING_NOFIFICATION_T2 64
#define SIZE_OF_SETTING_NOFIFICATION_TS 128
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
    esp3d_http_port,
    esp3d_http_on,
    esp3d_setup,
    esp3d_target_firmware,
    esp3d_check_update_on_sd,
    esp3d_notification_type,
    esp3d_auto_notification,
    esp3d_notification_token_1,
    esp3d_notification_token_2,
    esp3d_notification_token_setting,
    esp3d_socket_port,
    esp3d_socket_on,
    esp3d_ws_port,
    esp3d_ws_on,
    last_esp3d_setting_index_t
} esp3d_setting_index_t;

typedef enum  {
    esp3d_unknown           = 0,
    esp3d_grbl              = 10,
    esp3d_marlin            = 20,
    esp3d_marlin_embedded   = 30,
    esp3d_smoothieware      = 40,
    esp3d_repetier          = 50,
    esp3d_reprap            = 70,
    esp3d_grblhal           = 80,
    esp3d_hp_gl             = 90,
    last_esp3d_target_firmware_index_t
} esp3d_target_firmware_index_t;

typedef enum {
    esp3d_state_off=0,
    esp3d_state_on=1,
} esp3d_state_t;

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


typedef struct  {
    esp3d_setting_index_t index;
    esp3d_setting_type_t type;
    uint16_t size;
    const char* defaultval;
} esp3d_setting_desc_t;

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
    const esp3d_setting_desc_t * getSettingPtr(const esp3d_setting_index_t index);
    const char * GetFirmwareTargetShortName(esp3d_target_firmware_index_t index);
private:
    const char *IPUInt32toString(uint32_t ip_int);
    uint32_t StringtoIPUInt32(const char *s);

};

extern Esp3DSettings esp3dTFTsettings;

#ifdef __cplusplus
} // extern "C"
#endif