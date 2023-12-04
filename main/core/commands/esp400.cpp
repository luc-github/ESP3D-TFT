/*
  esp3d_commands member
  Copyright (c) 2022 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "translations/esp3d_translation_service.h"

#if ESP3D_SD_CARD_FEATURE
#include "sd_def.h"
#endif  // ESP3D_SD_CARD_FEATURE

#define COMMAND_ID 400
const char* BaudRateList[] = {"9600",   "19200",  "38400",  "57600",  "74880",
                              "115200", "230400", "250000", "500000", "921600"};
const char* SPIDivider[] = {"1", "2", "4", "6", "8", "16", "32"};
const char* YesNoLabels[] = {"no", "yes"};
const char* YesNoValues[] = {"0", "1"};
const char* RadioModeLabels[] = {"none", "sta", "ap", "setup"};
const char* RadioModeValues[] = {"0", "1", "2", "3"};
#if ESP3D_USB_SERIAL_FEATURE
const char* OutputClientsLabels[] = {
    "serial port",
    "usb port",
};
const char* OutputClientsValues[] = {"1", "2"};
#endif  // ESP3D_USB_SERIAL_FEATURE

const char* FirmwareLabels[] = {"Unknown",  "Grbl",    "Marlin", "Smoothieware",
                                "Repetier", "grblHAL", "HP_GL"};

const char* FirmwareValues[] = {"0", "10", "20", "40", "50", "80", "90"};
#if ESP3D_NOTIFICATIONS_FEATURE
const char* NotificationsLabels[] = {"none", "pushover", "email",
                                     "line", "telegram", "ifttt"};

const char* NotificationsValues[] = {"0", "1", "2", "3", "4", "5"};
#endif  // ESP3D_NOTIFICATIONS_FEATURE

const char* IpModeLabels[] = {"dhcp", "static"};
const char* IpModeValues[] = {"0", "1"};

const char* FallbackLabels[] = {"none", "setup"};
const char* FallbackValues[] = {"0", "3"};

const char* ApChannelsList[] = {"1", "2", "3",  "4",  "5",  "6",  "7",
                                "8", "9", "10", "11", "12", "13", "14"};

#define MIN_PORT_NUMBER 1
#define MAX_PORT_NUMBER 65535

// Get full ESP3D settings
//[ESP400]<pwd=admin>
void ESP3DCommands::ESP400(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;

  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (json) {
    tmpstr = "{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[";

  } else {
    tmpstr = "Settings:\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }

#if ESP3D_USB_SERIAL_FEATURE
  // output client (first item)
  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_output_client, "output",
                       OutputClientsValues, OutputClientsLabels,
                       sizeof(OutputClientsValues) / sizeof(char*), -1, -1, -1,
                       nullptr, true, target, requestId, true)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Baud rate (first item)
  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_baud_rate, "baud", BaudRateList,
                       BaudRateList, sizeof(BaudRateList) / sizeof(char*), -1,
                       -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_usb_serial_baud_rate,
                       "usb-serial baud", BaudRateList, BaudRateList,
                       sizeof(BaudRateList) / sizeof(char*), -1, -1, -1,
                       nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#else
  // Baud rate (first item)
  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_baud_rate, "baud", BaudRateList,
                       BaudRateList, sizeof(BaudRateList) / sizeof(char*), -1,
                       -1, -1, nullptr, true, target, requestId, true)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

  // Target Firmware
  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_target_firmware, "targetfw",
                       FirmwareValues, FirmwareLabels,
                       sizeof(FirmwareValues) / sizeof(char*), -1, -1, -1,
                       nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // TFT UI Language
  uint16_t count = esp3dTranslationService.getLanguagesList();
  char** UIValues = (char**)calloc(count, sizeof(char*));
  char** UILabels = (char**)calloc(count, sizeof(char*));
  if (UIValues && UILabels) {
    for (int i = 0; i < esp3dTranslationService.getLanguagesValues().size();
         i++) {
      UIValues[i] = (char*)calloc(
          (esp3dTranslationService.getLanguagesValues()[i].size() + 1),
          sizeof(char));
      if (UIValues[i])
        strcpy(UIValues[i],
               esp3dTranslationService.getLanguagesValues()[i].c_str());
    }
    for (int i = 0; i < esp3dTranslationService.getLanguagesLabels().size();
         i++) {
      UILabels[i] = (char*)calloc(
          (esp3dTranslationService.getLanguagesLabels()[i].size() + 1),
          sizeof(char));
      if (UILabels[i])
        strcpy(UILabels[i],
               esp3dTranslationService.getLanguagesLabels()[i].c_str());
    }
  }

  if (!dispatchSetting(json, "system/system",
                       ESP3DSettingIndex::esp3d_ui_language, "language",
                       (const char**)(UIValues), (const char**)(UILabels),
                       count, -1, -1, -1, nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  if (UIValues) {
    for (int i = 0; i < count; i++) {
      free(UIValues[i]);
    }
    free(UIValues);
  }
  if (UILabels) {
    for (int i = 0; i < count; i++) {
      free(UILabels[i]);
    }
    free(UILabels);
  }

  // Radio mode
  if (!dispatchSetting(json, "network/network",
                       ESP3DSettingIndex::esp3d_radio_mode, "radio mode",
                       RadioModeValues, RadioModeLabels,
                       sizeof(RadioModeValues) / sizeof(char*), -1, -1, -1,
                       nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Radio boot mode
  if (!dispatchSetting(json, "network/network",
                       ESP3DSettingIndex::esp3d_radio_mode, "radio_boot",
                       YesNoValues, YesNoLabels,
                       sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                       true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Hostname
  if (!dispatchSetting(json, "network/network",
                       ESP3DSettingIndex::esp3d_hostname, "hostname", nullptr,
                       nullptr, SIZE_OF_SETTING_HOSTNAME, 1, 1, -1, nullptr,
                       true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA SSID
  if (!dispatchSetting(json, "network/sta", ESP3DSettingIndex::esp3d_sta_ssid,
                       "SSID", nullptr, nullptr, SIZE_OF_SETTING_SSID_ID, 1, 1,
                       -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA Password
  if (!dispatchSetting(json, "network/sta",
                       ESP3DSettingIndex::esp3d_sta_password, "pwd", nullptr,
                       nullptr, SIZE_OF_SETTING_SSID_PWD, 8, 0, -1, nullptr,
                       true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA ip mode
  if (!dispatchSetting(
          json, "network/sta", ESP3DSettingIndex::esp3d_sta_ip_mode, "ip mode",
          IpModeValues, IpModeLabels, sizeof(IpModeLabels) / sizeof(char*), -1,
          -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA fallback mode
  if (!dispatchSetting(json, "network/sta",
                       ESP3DSettingIndex::esp3d_fallback_mode,
                       "sta fallback mode", FallbackValues, FallbackLabels,
                       sizeof(FallbackValues) / sizeof(char*), -1, -1, -1,
                       nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA static ip
  if (!dispatchSetting(
          json, "network/sta", ESP3DSettingIndex::esp3d_sta_ip_static, "ip",
          nullptr, nullptr, -1, -1, -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA static mask
  if (!dispatchSetting(
          json, "network/sta", ESP3DSettingIndex::esp3d_sta_mask_static, "msk",
          nullptr, nullptr, -1, -1, -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA static gateway
  if (!dispatchSetting(
          json, "network/sta", ESP3DSettingIndex::esp3d_sta_gw_static, "gw",
          nullptr, nullptr, -1, -1, -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // STA static dns
  if (!dispatchSetting(
          json, "network/sta", ESP3DSettingIndex::esp3d_sta_dns_static, "DNS",
          nullptr, nullptr, -1, -1, -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // AP SSID
  if (!dispatchSetting(json, "network/ap", ESP3DSettingIndex::esp3d_ap_ssid,
                       "SSID", nullptr, nullptr, SIZE_OF_SETTING_SSID_ID, 1, 1,
                       -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // AP Password
  if (!dispatchSetting(json, "network/ap", ESP3DSettingIndex::esp3d_ap_password,
                       "pwd", nullptr, nullptr, SIZE_OF_SETTING_SSID_PWD, 8, 0,
                       -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // AP static ip
  if (!dispatchSetting(
          json, "network/ap", ESP3DSettingIndex::esp3d_ap_ip_static, "ip",
          nullptr, nullptr, -1, -1, -1, -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // AP Channel
  if (!dispatchSetting(json, "network/ap", ESP3DSettingIndex::esp3d_ap_channel,
                       "channel", ApChannelsList, ApChannelsList,
                       sizeof(ApChannelsList) / sizeof(char*), -1, -1, -1,
                       nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#if ESP3D_AUTHENTICATION_FEATURE
  // Session timeout
  if (!dispatchSetting(json, "security/security",
                       ESP3DSettingIndex::esp3d_session_timeout,
                       "session timeout", nullptr, nullptr, 255, 0, -1, -1,
                       nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Admin Password
  if (!dispatchSetting(json, "security/security",
                       ESP3DSettingIndex::esp3d_admin_password, "admin pwd",
                       nullptr, nullptr, SIZE_OF_LOCAL_PASSWORD, 0, -1, -1,
                       nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // User Password
  if (!dispatchSetting(json, "security/security",
                       ESP3DSettingIndex::esp3d_user_password, "user pwd",
                       nullptr, nullptr, SIZE_OF_LOCAL_PASSWORD, 0, -1, -1,
                       nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_HTTP_FEATURE
  // Http service on
  if (!dispatchSetting(json, "service/http", ESP3DSettingIndex::esp3d_http_on,
                       "enable", YesNoValues, YesNoLabels,
                       sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                       true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Http port
  if (!dispatchSetting(json, "service/http", ESP3DSettingIndex::esp3d_http_port,
                       "port", nullptr, nullptr, MAX_PORT_NUMBER,
                       MIN_PORT_NUMBER, -1, -1, nullptr, true, target,
                       requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#if ESP3D_WEBDAV_SERVICES_FEATURE
  // webdav service on
  if (!dispatchSetting(
          json, "service/webdavp", ESP3DSettingIndex::esp3d_webdav_on, "enable",
          YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1,
          -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#endif  // ESP3D_HTTP_FEATURE

#if ESP3D_TELNET_FEATURE
  // Socket/Telnet
  if (!dispatchSetting(
          json, "service/telnetp", ESP3DSettingIndex::esp3d_socket_on, "enable",
          YesNoValues, YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1,
          -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Socket/Telnet port
  if (!dispatchSetting(json, "service/telnetp",
                       ESP3DSettingIndex::esp3d_socket_port, "port", nullptr,
                       nullptr, MAX_PORT_NUMBER, MIN_PORT_NUMBER, -1, -1,
                       nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_TELNET_FEATURE

  // WebSocket
  if (!dispatchSetting(json, "service/websocketp",
                       ESP3DSettingIndex::esp3d_ws_on, "enable", YesNoValues,
                       YesNoLabels, sizeof(YesNoValues) / sizeof(char*), -1, -1,
                       -1, nullptr, true, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#if ESP3D_NOTIFICATIONS_FEATURE
  // Notifications type
  if (!dispatchSetting(json, "service/notification",
                       ESP3DSettingIndex::esp3d_notification_type,
                       "notification", NotificationsValues, NotificationsLabels,
                       sizeof(NotificationsValues) / sizeof(char*), -1, -1, -1,
                       nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Auto notifications
  if (!dispatchSetting(json, "service/notification",
                       ESP3DSettingIndex::esp3d_auto_notification, "auto notif",
                       YesNoValues, YesNoLabels,
                       sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Notification token 1
  if (!dispatchSetting(json, "service/notification",
                       ESP3DSettingIndex::esp3d_notification_token_1, "t1",
                       nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_T1, 0, 0,
                       -1, nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Notification token 2
  if (!dispatchSetting(json, "service/notification",
                       ESP3DSettingIndex::esp3d_notification_token_2, "t2",
                       nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_T2, 0, 0,
                       -1, nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Notification token setting
  if (!dispatchSetting(json, "service/notification",
                       ESP3DSettingIndex::esp3d_notification_token_setting,
                       "ts", nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_TS,
                       0, 0, -1, nullptr, false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_NOTIFICATIONS_FEATURE
  // Gcode host pause script
  if (!dispatchSetting(json, "service/gcodehost",
                       ESP3DSettingIndex::esp3d_pause_script, "pause_script",
                       nullptr, nullptr, SIZE_OF_SCRIPT, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Gcode host resume script
  if (!dispatchSetting(json, "service/gcodehost",
                       ESP3DSettingIndex::esp3d_resume_script, "resume_script",
                       nullptr, nullptr, SIZE_OF_SCRIPT, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Gcode host stop script
  if (!dispatchSetting(json, "service/gcodehost",
                       ESP3DSettingIndex::esp3d_stop_script, "stop_script",
                       nullptr, nullptr, SIZE_OF_SCRIPT, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_IS_SPI
  // SPI Divider factor
  if (!dispatchSetting(json, "device/sd", ESP3DSettingIndex::esp3d_spi_divider,
                       "speedx", SPIDivider, SPIDivider,
                       sizeof(SPIDivider) / sizeof(char*), -1, -1, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_SD_IS_SPI
#if ESP3D_UPDATE_FEATURE
  // SD updater
  if (!dispatchSetting(json, "device/sd",
                       ESP3DSettingIndex::esp3d_check_update_on_sd,
                       "SD updater", YesNoValues, YesNoLabels,
                       sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
  // Use internet time
  if (!dispatchSetting(json, "service/time",
                       ESP3DSettingIndex::esp3d_use_internet_time, "i-time",
                       YesNoValues, YesNoLabels,
                       sizeof(YesNoValues) / sizeof(char*), -1, -1, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Timezone
  if (!dispatchSetting(json, "service/time", ESP3DSettingIndex::esp3d_timezone,
                       "tzone", SupportedTimeZones, SupportedTimeZones,
                       SupportedTimeZonesSize, -1, -1, -1, nullptr, false,
                       target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }

  // Time server 1
  if (!dispatchSetting(json, "service/time",
                       ESP3DSettingIndex::esp3d_time_server1, "t-server",
                       nullptr, nullptr, SIZE_OF_SERVER_URL, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Time server 2
  if (!dispatchSetting(json, "service/time",
                       ESP3DSettingIndex::esp3d_time_server2, "t-server",
                       nullptr, nullptr, SIZE_OF_SERVER_URL, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
  // Time server 3
  if (!dispatchSetting(json, "service/time",
                       ESP3DSettingIndex::esp3d_time_server3, "t-server",
                       nullptr, nullptr, SIZE_OF_SERVER_URL, 0, 0, -1, nullptr,
                       false, target, requestId)) {
    esp3d_log_e("Error sending response to clients");
  }
#endif  // ESP3D_TIMESTAMP_FEATURE
  if (json) {
    if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending response to clients");
    }
  } else {
    if (!dispatch("ok\n", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending response to clients");
    }
  }
}
