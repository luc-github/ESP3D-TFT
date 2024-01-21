
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
#include "nvs.h"

const uint32_t SupportedBaudList[] = {9600,   19200,  38400,  57600,  74880,
                                      115200, 230400, 250000, 500000, 921600};
#if ESP3D_TIMESTAMP_FEATURE
extern const uint8_t SupportedTimeZonesSize;
extern const char* SupportedTimeZones[];
#define SIZE_OF_SERVER_URL 128
#define SIZE_OF_TIMEZONE 7
#endif  // ESP3D_TIMESTAMP_FEATURE

#define SIZE_OF_SCRIPT 255
#define SIZE_OF_SETTING_VERSION 25
#define SIZE_OF_SETTING_SSID_ID 32
#define SIZE_OF_SETTING_SSID_PWD 64
#define SIZE_OF_SETTING_HOSTNAME 32
#define SIZE_OF_UI_LANGUAGE 30
#if ESP3D_NOTIFICATIONS_FEATURE
#define SIZE_OF_SETTING_NOFIFICATION_T1 64
#define SIZE_OF_SETTING_NOFIFICATION_T2 64
#define SIZE_OF_SETTING_NOFIFICATION_TS 128
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#define SIZE_OF_LOCAL_PASSWORD 20
#define HIDDEN_SETTING_VALUE "********"
// do not change the order of the enum
// using #if to keep consistency if user update feature
enum class ESP3DSettingIndex : uint16_t {
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
  esp3d_ws_on,
  esp3d_usb_serial_baud_rate,
  esp3d_output_client,
  esp3d_admin_password,
  esp3d_user_password,
  esp3d_session_timeout,
  esp3d_ui_language,
  esp3d_jog_type,
  esp3d_polling_on,
  esp3d_auto_level_on,
  esp3d_bed_width,
  esp3d_bed_depth,
  esp3d_inverved_x,
  esp3d_inverved_y,
  esp3d_extensions,         // json setting, in preferences.json
  esp3d_show_fan_controls,  // json setting, in preferences.json
  esp3d_pause_script,
  esp3d_stop_script,
  esp3d_resume_script,
  esp3d_use_internet_time,
  esp3d_time_server1,
  esp3d_time_server2,
  esp3d_time_server3,
  esp3d_timezone,
  esp3d_webdav_on,
  unknown_index
};

enum class ESP3DTargetFirmware : uint8_t {
  unknown = 0,
  grbl = 10,
  marlin = 20,
  marlin_embedded = 30,
  smoothieware = 40,
  repetier = 50,
  reprap = 70,
  grblhal = 80,
  hp_gl = 90,
};

enum class ESP3DState : uint8_t {
  off = 0,
  on = 1,
};

enum class ESP3DSettingType : uint8_t {
  byte_t,     // byte
  integer_t,  // 4 bytes
  string_t,   // string
  ip_t,       // 4 bytes
  float_t,    // 4 bytes
  mask,       // x bytes
  bitsfield,  // x bytes
  unknown_t
};

struct ESP3DSettingDescription {
  ESP3DSettingIndex index;
  ESP3DSettingType type;
  uint16_t size;
  const char* default_val;
};

class ESP3DSettings final {
 public:
  ESP3DSettings();
  ~ESP3DSettings();
  bool isValidSettingsNvs();
  bool access_nvs(nvs_open_mode_t mode);
  void release_nvs(nvs_open_mode_t mode);
  uint8_t readByte(ESP3DSettingIndex index, bool* haserror = NULL);
  uint32_t readUint32(ESP3DSettingIndex index, bool* haserror = NULL);
  const char* readIPString(ESP3DSettingIndex index, bool* haserror = NULL);
  const char* readString(ESP3DSettingIndex index, char* out_str, size_t len,
                         bool* haserror = NULL);
  bool writeByte(ESP3DSettingIndex index, const uint8_t value);
  bool writeUint32(ESP3DSettingIndex index, const uint32_t value);
  bool writeIPString(ESP3DSettingIndex index, const char* byte_buffer);
  bool writeString(ESP3DSettingIndex index, const char* byte_buffer);
  bool reset();
  bool isValidIPStringSetting(const char* value,
                              ESP3DSettingIndex settingElement);
  bool isValidStringSetting(const char* value,
                            ESP3DSettingIndex settingElement);
  bool isValidIntegerSetting(uint32_t value, ESP3DSettingIndex settingElement);
  bool isValidByteSetting(uint8_t value, ESP3DSettingIndex settingElement);
  uint32_t getDefaultIntegerSetting(ESP3DSettingIndex settingElement);
  const char* getDefaultStringSetting(ESP3DSettingIndex settingElement);
  uint8_t getDefaultByteSetting(ESP3DSettingIndex settingElement);
  const ESP3DSettingDescription* getSettingPtr(const ESP3DSettingIndex index);
  const char* GetFirmwareTargetShortName(ESP3DTargetFirmware index);

 private:
  const char* IPUInt32toString(uint32_t ip_int);
  uint32_t StringtoIPUInt32(const char* s);
};

extern ESP3DSettings esp3dTftsettings;

#ifdef __cplusplus
}  // extern "C"
#endif
