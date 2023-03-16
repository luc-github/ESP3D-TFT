
/*
  esp3d_settings.cpp -  settings esp3d functions class

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
#include "esp3d_settings.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"

#if ESP3D_WIFI_FEATURE
#include "lwip/ip_addr.h"
#endif  // ESP3D_WIFI_FEATURE
#include <cstring>
#include <regex>
#include <string>

#include "esp3d_client.h"
#include "esp3d_log.h"
#include "esp_system.h"
#include "serial_def.h"

#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial_def.h"
#endif  // ESP3D_USB_SERIAL_FEATURE
#include "network/esp3d_network.h"
#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif  // ESP3D_NOTIFICATIONS_FEATURE

#include "authentication/esp3d_authentication.h"

#define STORAGE_NAME "ESP3D_TFT"
#define SETTING_VERSION "ESP3D_TFT-V1.0.0"

ESP3dSettings esp3dTFTsettings;

const uint32_t SupportedBaudList[] = {9600,   19200,  38400,  57600,  74880,
                                      115200, 230400, 250000, 500000, 921600};
const uint8_t SupportedBaudListSize =
    sizeof(SupportedBaudList) / sizeof(uint32_t);

const uint8_t SupportedSPIDivider[] = {1, 2, 4, 6, 8, 16, 32};
const uint8_t SupportedSPIDividerSize =
    sizeof(SupportedSPIDivider) / sizeof(uint8_t);

const uint8_t SupportedApChannels[] = {1, 2, 3,  4,  5,  6,  7,
                                       8, 9, 10, 11, 12, 13, 14};
const uint8_t SupportedApChannelsSize =
    sizeof(SupportedApChannels) / sizeof(uint8_t);

// value of settings, default value are all strings
const Esp3dSettingDescription ESP3dSettingsData[] = {
    {Esp3dSettingIndex::esp3d_version, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_VERSION, "Invalid data"},  // Version
    {Esp3dSettingIndex::esp3d_baud_rate, Esp3dSettingType::integer_t, 4,
     ESP3D_SERIAL_BAUDRATE},  // BaudRate
#if ESP3D_USB_SERIAL_FEATURE
    {Esp3dSettingIndex::esp3d_usb_serial_baud_rate, Esp3dSettingType::integer_t,
     4, ESP3D_USB_SERIAL_BAUDRATE},  // BaudRate
    {Esp3dSettingIndex::esp3d_output_client, Esp3dSettingType::byte_t, 1, "1"},
#endif  // ESP3D_USB_SERIAL_FEATURE
    {Esp3dSettingIndex::esp3d_hostname, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_HOSTNAME, "esp3d-tft"},
    {Esp3dSettingIndex::esp3d_radio_boot_mode, Esp3dSettingType::byte_t, 1,
     "1"},
    {Esp3dSettingIndex::esp3d_radio_mode, Esp3dSettingType::byte_t, 1, "3"},
#if ESP3D_WIFI_FEATURE
    {Esp3dSettingIndex::esp3d_fallback_mode, Esp3dSettingType::byte_t, 1, "3"},
    {Esp3dSettingIndex::esp3d_sta_ssid, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_SSID_ID, ""},
    {Esp3dSettingIndex::esp3d_sta_password, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_SSID_PWD, ""},
    {Esp3dSettingIndex::esp3d_sta_ip_mode, Esp3dSettingType::byte_t, 1, "0"},
    {Esp3dSettingIndex::esp3d_sta_ip_static, Esp3dSettingType::ip, 4,
     "192.168.1.100"},
    {Esp3dSettingIndex::esp3d_sta_mask_static, Esp3dSettingType::ip, 4,
     "255.255.255.0"},
    {Esp3dSettingIndex::esp3d_sta_gw_static, Esp3dSettingType::ip, 4,
     "192.168.1.1"},
    {Esp3dSettingIndex::esp3d_sta_dns_static, Esp3dSettingType::ip, 4,
     "192.168.1.1"},
    {Esp3dSettingIndex::esp3d_ap_ssid, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_SSID_ID, "esp3dtft"},
    {Esp3dSettingIndex::esp3d_ap_password, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_SSID_PWD, "12345678"},
    {Esp3dSettingIndex::esp3d_ap_ip_static, Esp3dSettingType::ip, 4,
     "192.168.0.1"},
    {Esp3dSettingIndex::esp3d_ap_channel, Esp3dSettingType::byte_t, 1, "2"},
#endif  // ESP3D_WIFI_FEATURE
#if ESP3D_HTTP_FEATURE
    {Esp3dSettingIndex::esp3d_http_port, Esp3dSettingType::integer_t, 4, "80"},
    {Esp3dSettingIndex::esp3d_http_on, Esp3dSettingType::byte_t, 1, "1"},
#endif  // ESP3D_HTTP_FEATURE
    {Esp3dSettingIndex::esp3d_setup, Esp3dSettingType::byte_t, 1, "0"},
    {Esp3dSettingIndex::esp3d_target_firmware, Esp3dSettingType::byte_t, 1,
     "0"},
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_FEATURE_IS_SPI
    {Esp3dSettingIndex::esp3d_spi_divider, Esp3dSettingType::byte_t, 1,
     "1"},  // SPIdivider
#endif      // ESP3D_SD_FEATURE_IS_SPI
#if ESP3D_UPDATE_FEATURE
    {Esp3dSettingIndex::esp3d_check_update_on_sd, Esp3dSettingType::byte_t, 1,
     "1"},
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    {Esp3dSettingIndex::esp3d_notification_type, Esp3dSettingType::byte_t, 1,
     "0"},
    {Esp3dSettingIndex::esp3d_auto_notification, Esp3dSettingType::byte_t, 1,
     "0"},
    {Esp3dSettingIndex::esp3d_notification_token_1, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_NOFIFICATION_T1, ""},
    {Esp3dSettingIndex::esp3d_notification_token_2, Esp3dSettingType::string_t,
     SIZE_OF_SETTING_NOFIFICATION_T2, ""},
    {Esp3dSettingIndex::esp3d_notification_token_setting,
     Esp3dSettingType::string_t, SIZE_OF_SETTING_NOFIFICATION_TS, ""},
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_TELNET_FEATURE
    {Esp3dSettingIndex::esp3d_socket_port, Esp3dSettingType::integer_t, 4,
     "23"},
    {Esp3dSettingIndex::esp3d_socket_on, Esp3dSettingType::byte_t, 1, "1"},
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    {Esp3dSettingIndex::esp3d_ws_on, Esp3dSettingType::byte_t, 1, "1"},
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    {Esp3dSettingIndex::esp3d_admin_password, Esp3dSettingType::string_t,
     SIZE_OF_LOCAL_PASSWORD, "admin"},
    {Esp3dSettingIndex::esp3d_user_password, Esp3dSettingType::string_t,
     SIZE_OF_LOCAL_PASSWORD, "user"},
    {Esp3dSettingIndex::esp3d_session_timeout, Esp3dSettingType::byte_t, 1,
     "3"},
#endif  // ESP3D_AUTHENTICATION_FEATURE
};

bool ESP3dSettings::isValidStringSetting(const char* value,
                                         Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (!(settingPtr->type == Esp3dSettingType::string_t)) {
    return false;
  }
  if (strlen(value) > settingPtr->size) {
    return false;
  }
  // use strlen because it crash with regex if value is longer than 41
  // characters
#if ESP3D_WIFI_FEATURE
  size_t len = strlen(value);
#endif  // ESP3D_WIFI_FEATURE
  switch (settingElement) {
#if ESP3D_WIFI_FEATURE
    case Esp3dSettingIndex::esp3d_ap_ssid:
    case Esp3dSettingIndex::esp3d_sta_ssid:
      return (len > 0 &&
              len <= SIZE_OF_SETTING_SSID_ID);  // any string from 1 to 32
    case Esp3dSettingIndex::esp3d_sta_password:
    case Esp3dSettingIndex::esp3d_ap_password:
      return (
          len == 0 ||
          (len >= 8 &&
           len <= SIZE_OF_SETTING_SSID_PWD));  // any string from 8 to 64 or 0
#endif                                         // ESP3D_WIFI_FEATURE
    case Esp3dSettingIndex::esp3d_hostname:
      esp3d_log("Checking hostname validity");
      return std::regex_match(
          value,
          std::regex(
              "^[a-zA-Z0-9]{1}[a-zA-Z0-9\\-]{0,31}$"));  // any string
                                                         // alphanumeric or '-'
                                                         // from 1 to 32
#if ESP3D_NOTIFICATIONS_FEATURE
    case Esp3dSettingIndex::esp3d_notification_token_1:
      return len <= SIZE_OF_SETTING_NOFIFICATION_T1;  // any string from 0 to 64
    case Esp3dSettingIndex::esp3d_notification_token_2:
      return len <= SIZE_OF_SETTING_NOFIFICATION_T2;  // any string from 0 to 64
    case Esp3dSettingIndex::esp3d_notification_token_setting:
      return len <= SIZE_OF_SETTING_NOFIFICATION_TS;  // any string from 0 to
                                                      // 128
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    case Esp3dSettingIndex::esp3d_admin_password:
    case Esp3dSettingIndex::esp3d_user_password:
      return len <= SIZE_OF_LOCAL_PASSWORD;  // any string from 0 to 20
#endif                                       // ESP3D_AUTHENTICATION_FEATURE
    default:
      return false;
  }
  return false;
}

bool ESP3dSettings::isValidIntegerSetting(uint32_t value,
                                          Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (!(settingPtr->type == Esp3dSettingType::integer_t ||
        settingPtr->type == Esp3dSettingType::ip)) {
    return false;
  }
  switch (settingElement) {
#if ESP3D_USB_SERIAL_FEATURE
    case Esp3dSettingIndex::esp3d_usb_serial_baud_rate:
#endif  // #if ESP3D_USB_SERIAL_FEATURE
    case Esp3dSettingIndex::esp3d_baud_rate:
      for (uint8_t i = 0; i < SupportedBaudListSize; i++) {
        if (SupportedBaudList[i] == value) {
          return true;
        }
      }
      break;
#if ESP3D_TELNET_FEATURE
    case Esp3dSettingIndex::esp3d_socket_port:
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_HTTP_FEATURE
    case Esp3dSettingIndex::esp3d_http_port:
      if (value >= 1 && value < 65535) {
        return true;
      }
      break;
#endif  // ESP3D_HTTP_FEATURE

    default:
      return false;
  }
  return false;
}

bool ESP3dSettings::isValidByteSetting(uint8_t value,
                                       Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (settingPtr->type != Esp3dSettingType::byte_t) {
    return false;
  }
  switch (settingElement) {
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
    case Esp3dSettingIndex::esp3d_check_update_on_sd:
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE

    case Esp3dSettingIndex::esp3d_setup:
#if ESP3D_TELNET_FEATURE
    case Esp3dSettingIndex::esp3d_socket_on:
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    case Esp3dSettingIndex::esp3d_ws_on:
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_HTTP_FEATURE
    case Esp3dSettingIndex::esp3d_http_on:
#endif  // ESP3D_HTTP_FEATURE
    case Esp3dSettingIndex::esp3d_radio_boot_mode:
#if ESP3D_NOTIFICATIONS_FEATURE
    case Esp3dSettingIndex::esp3d_auto_notification:
#endif  // ESP3D_NOTIFICATIONS_FEATURE
      if (value == (uint8_t)Esp3dState::off ||
          value == (uint8_t)Esp3dState::on) {
        return true;
      }
      break;
#if ESP3D_AUTHENTICATION_FEATURE
    case Esp3dSettingIndex::esp3d_session_timeout:
      return true;  // 0 ->255 minutes
      break;
#endif  // ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_USB_SERIAL_FEATURE
    case Esp3dSettingIndex::esp3d_output_client:
      return ((Esp3dClientType)value == Esp3dClientType::serial ||
              (Esp3dClientType)value == Esp3dClientType::usb_serial);
      break;
#endif  // #if ESP3D_USB_SERIAL_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    case Esp3dSettingIndex::esp3d_notification_type:
      if (value == static_cast<uint8_t>(Esp3dNotificationType::none) ||
          value == static_cast<uint8_t>(Esp3dNotificationType::pushover) ||
          value == static_cast<uint8_t>(Esp3dNotificationType::email) ||
          value == static_cast<uint8_t>(Esp3dNotificationType::line) ||
          value == static_cast<uint8_t>(Esp3dNotificationType::telegram) ||
          value == static_cast<uint8_t>(Esp3dNotificationType::ifttt)) {
        return true;
      }
      break;
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_WIFI_FEATURE
    case Esp3dSettingIndex::esp3d_sta_ip_mode:
      if (value == static_cast<uint8_t>(Esp3dIpMode::dhcp) ||
          value == static_cast<uint8_t>(Esp3dIpMode::staticIp)) {
        return true;
      }
      break;
    case Esp3dSettingIndex::esp3d_fallback_mode:
      if (value == (uint8_t)Esp3dRadioMode::off ||
          value == (uint8_t)Esp3dRadioMode::wifi_ap_config ||
          value == (uint8_t)Esp3dRadioMode::bluetooth_serial) {
        return true;
      }
      break;
#endif  // ESP3D_WIFI_FEATURE
    case Esp3dSettingIndex::esp3d_radio_mode:
      if (value == (uint8_t)Esp3dRadioMode::off
#if ESP3D_WIFI_FEATURE
          || value == (uint8_t)Esp3dRadioMode::wifi_sta ||
          value == (uint8_t)Esp3dRadioMode::wifi_ap ||
          value == (uint8_t)Esp3dRadioMode::wifi_ap_config
#endif  // ESP3D_WIFI_FEATURE
          || value == (uint8_t)Esp3dRadioMode::bluetooth_serial) {
        return true;
      }
      break;
    case Esp3dSettingIndex::esp3d_ap_channel:
      for (uint8_t i = 0; i < SupportedApChannelsSize; i++) {
        if (SupportedApChannels[i] == value) {
          return true;
        }
      }
      break;
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_FEATURE_IS_SPI
    case Esp3dSettingIndex::esp3d_spi_divider:
      for (uint8_t i = 0; i < SupportedSPIDividerSize; i++) {
        if (SupportedSPIDivider[i] == value) {
          return true;
        }
      }
      break;
#endif  // ESP3D_SD_FEATURE_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE

    case Esp3dSettingIndex::esp3d_target_firmware:
      if (static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::unknown ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::grbl ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::marlin ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::marlin_embedded ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::smoothieware ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::repetier ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::reprap ||
          static_cast<Esp3dTargetFirmware>(value) ==
              Esp3dTargetFirmware::grblhal ||
          static_cast<Esp3dTargetFirmware>(value) == Esp3dTargetFirmware::hp_gl)
        return true;

      break;
    default:
      break;
  }
  return false;
}
bool ESP3dSettings::isValidIPStringSetting(const char* value,
                                           Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* settingPtr = getSettingPtr(settingElement);
  if (!settingPtr) {
    return false;
  }
  if (settingPtr->type != Esp3dSettingType::ip) {
    return false;
  }
  return std::regex_match(value,
                          std::regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"));
}

bool ESP3dSettings::isValidSettingsNvs() {
  char result[SIZE_OF_SETTING_VERSION + 1] = {0};
  if (esp3dTFTsettings.readString(Esp3dSettingIndex::esp3d_version, result,
                                  SIZE_OF_SETTING_VERSION + 1)) {
    if (strcmp(SETTING_VERSION, result) != 0) {
      esp3d_log_e("Expected %s but got %s", SETTING_VERSION, result);
      return false;
    } else {
      return true;
    }
  } else {
    esp3d_log_e("Cannot read setting version");
    return false;
  }
}

uint32_t ESP3dSettings::getDefaultIntegerSetting(
    Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* query = getSettingPtr(settingElement);
  if (query) {
    return (uint32_t)std::stoul(std::string(query->default_val), NULL, 0);
  }
  return 0;
}
const char* ESP3dSettings::getDefaultStringSetting(
    Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* query = getSettingPtr(settingElement);
  if (query) {
    return query->default_val;
  }
  return nullptr;
}
uint8_t ESP3dSettings::getDefaultByteSetting(Esp3dSettingIndex settingElement) {
  const Esp3dSettingDescription* query = getSettingPtr(settingElement);
  if (query) {
    return (uint8_t)std::stoul(std::string(query->default_val), NULL, 0);
  }
  return 0;
}

const char* ESP3dSettings::GetFirmwareTargetShortName(
    Esp3dTargetFirmware index) {
  switch (index) {
    case Esp3dTargetFirmware::grbl:
      return "grbl";
    case Esp3dTargetFirmware::marlin:
      return "marlin";
    case Esp3dTargetFirmware::marlin_embedded:
      return "marlin";
    case Esp3dTargetFirmware::smoothieware:
      return "smoothieware";
    case Esp3dTargetFirmware::repetier:
      return "repetier";
    case Esp3dTargetFirmware::reprap:
      return "reprap";
    case Esp3dTargetFirmware::grblhal:
      return "grblhal";
    case Esp3dTargetFirmware::hp_gl:
      return "hp_gl";
    default:
      break;
  }
  return "unknown";
}

bool ESP3dSettings::reset() {
  esp3d_log("Resetting NVS");
  bool result = true;
  esp_err_t err;
  // clear all settings first
  nvs_handle_t handle_erase;
  err = nvs_open(STORAGE_NAME, NVS_READWRITE, &handle_erase);
  if (err != ESP_OK) {
    esp3d_log_e("Failing accessing NVS");
    return false;
  }
  if (nvs_erase_all(handle_erase) != ESP_OK) {
    esp3d_log_e("Failing erasing NVS");
    return false;
  }
  nvs_close(handle_erase);

  // Init each setting with default value
  // this parsing method is to workaround parsing array of enums and usage of
  // sizeof(<array of enums>) generate warning: iteration 1 invokes undefined
  // behavior [-Waggressive-loop-optimizations] may be use a vector of enum
  // would solve also fortunaly this is to reset all settings so it is ok for
  // current situation and it avoid to create an array of enums
  for (auto i : ESP3dSettingsData) {
    Esp3dSettingIndex setting = i.index;
    const Esp3dSettingDescription* query = getSettingPtr(setting);
    if (query) {
      esp3d_log("Reseting %d value to %s", static_cast<uint16_t>(setting),
                query->default_val);
      switch (query->type) {
        case Esp3dSettingType::byte_t:
          if (!ESP3dSettings::writeByte(
                  setting, (uint8_t)std::stoul(std::string(query->default_val),
                                               NULL, 0))) {
            esp3d_log_e("Error writing %s to settings %d", query->default_val,
                        static_cast<uint16_t>(setting));
            result = false;
          }
          break;
        case Esp3dSettingType::ip:
          if (!ESP3dSettings::writeIPString(setting, query->default_val)) {
            esp3d_log_e("Error writing %s to settings %d", query->default_val,
                        static_cast<uint16_t>(setting));
            result = false;
          }
          break;
        case Esp3dSettingType::integer_t:
          if (!ESP3dSettings::writeUint32(
                  setting, (uint32_t)std::stoul(std::string(query->default_val),
                                                NULL, 0))) {
            esp3d_log_e("Error writing %s to settings %d", query->default_val,
                        static_cast<uint16_t>(setting));
            result = false;
          }
          break;
        case Esp3dSettingType::string_t:
          if (setting == Esp3dSettingIndex::esp3d_version) {
            if (!ESP3dSettings::writeString(setting, SETTING_VERSION)) {
              esp3d_log_e("Error writing %s to settings %d", query->default_val,
                          static_cast<uint16_t>(setting));
              result = false;
            }
          } else {
            if (!ESP3dSettings::writeString(setting, query->default_val)) {
              esp3d_log_e("Error writing %s to settings %d", query->default_val,
                          static_cast<uint16_t>(setting));
              result = false;
            }
          }

          break;
        default:
          result = false;  // type is not handle
          esp3d_log_e("Setting  %d , type %d is not handled ",
                      static_cast<uint16_t>(setting),
                      static_cast<uint8_t>(query->type));
      }
    } else {
      result = false;  // setting invalid
      esp3d_log_e("Setting  %d is unknown", static_cast<uint16_t>(setting));
    }
  }
  return result;
}

ESP3dSettings::ESP3dSettings() {}

ESP3dSettings::~ESP3dSettings() {}

uint8_t ESP3dSettings::readByte(Esp3dSettingIndex index, bool* haserror) {
  uint8_t value = 0;
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::byte_t) {
      esp_err_t err;
      std::shared_ptr<nvs::NVSHandle> handle =
          nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
      if (err != ESP_OK) {
        esp3d_log_e("Failling accessing NVS");
      } else {
        std::string key = "p_" + std::to_string((uint)query->index);
        err = handle->get_item(key.c_str(), value);
        if (err == ESP_OK) {
          if (haserror) {
            *haserror = false;
          }
          return value;
        }
        if (err == ESP_ERR_NVS_NOT_FOUND) {
          value = (uint8_t)std::stoul(std::string(query->default_val), NULL, 0);
          if (haserror) {
            *haserror = false;
          }
          return value;
        }
      }
    }
  }
  if (haserror) {
    *haserror = true;
  }
  return 0;
}

uint32_t ESP3dSettings::readUint32(Esp3dSettingIndex index, bool* haserror) {
  uint32_t value = 0;
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::integer_t ||
        query->type == Esp3dSettingType::ip) {
      esp_err_t err;
      std::shared_ptr<nvs::NVSHandle> handle =
          nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
      if (err != ESP_OK) {
        esp3d_log_e("Failling accessing NVS");
      } else {
        std::string key = "p_" + std::to_string((uint)query->index);
        err = handle->get_item(key.c_str(), value);
        if (err == ESP_OK) {
          if (haserror) {
            *haserror = false;
          }
          return value;
        }
        if (err == ESP_ERR_NVS_NOT_FOUND) {
          if (query->type == Esp3dSettingType::integer_t) {
            value =
                (uint32_t)std::stoul(std::string(query->default_val), NULL, 0);
          } else {  // ip is stored as uin32t but default value is ip format
                    // string
            value = StringtoIPUInt32(query->default_val);
          }

          if (haserror) {
            *haserror = false;
          }
          return value;
        }
      }
    }
  }
  if (haserror) {
    *haserror = true;
  }
  return 0;
}

const char* ESP3dSettings::readIPString(Esp3dSettingIndex index,
                                        bool* haserror) {
  uint32_t ipInt = readUint32(index, haserror);
  std::string ipStr = IPUInt32toString(ipInt);
  // esp3d_log("read setting %d : %d to %s",index, ipInt,ipStr.c_str());
  return IPUInt32toString(ipInt);
}

const char* ESP3dSettings::readString(Esp3dSettingIndex index, char* out_str,
                                      size_t len, bool* haserror) {
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::string_t) {
      esp_err_t err;
      if (out_str && len >= (query->size)) {
        std::shared_ptr<nvs::NVSHandle> handle =
            nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
        if (err != ESP_OK) {
          esp3d_log_e("Failling accessing NVS");
        } else {
          std::string key = "p_" + std::to_string((uint)query->index);
          err = handle->get_string(key.c_str(), out_str, query->size);
          if (err == ESP_OK) {
            if (haserror) {
              *haserror = false;
            }
            return out_str;
          } else {
            esp3d_log_w("Got error %d %s", err, esp_err_to_name(err));
            if (err == ESP_ERR_NVS_NOT_FOUND) {
              esp3d_log_w("Not found value for %s, use default %s", key.c_str(),
                          query->default_val);
              strcpy(out_str, query->default_val);
              if (haserror) {
                *haserror = false;
              }
              return out_str;
            }
          }
          esp3d_log_e("Read %s failed: %s", key.c_str(), esp_err_to_name(err));
        }
      } else {
        if (!out_str) {
          esp3d_log_e("Error no output buffer");
        } else {
          esp3d_log_e("Error size are not correct got %d but should be %d", len,
                      query->size);
        }
      }
    } else {
      esp3d_log_e("Error setting is not a string");
    }
  } else {
    esp3d_log_e("Cannot find %d entry", static_cast<uint16_t>(index));
  }
  if (haserror) {
    *haserror = true;
  }
  esp3d_log_e("Error reading setting value");
  return "";
}

bool ESP3dSettings::writeByte(Esp3dSettingIndex index, const uint8_t value) {
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::byte_t) {
      esp_err_t err;
      std::shared_ptr<nvs::NVSHandle> handle =
          nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
      if (err != ESP_OK) {
        esp3d_log_e("Failling accessing NVS");
      } else {
        std::string key = "p_" + std::to_string((uint)query->index);
        if (handle->set_item(key.c_str(), value) == ESP_OK) {
          return (handle->commit() == ESP_OK);
        }
      }
    }
  }
  return false;
}

bool ESP3dSettings::writeUint32(Esp3dSettingIndex index, const uint32_t value) {
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::integer_t ||
        query->type == Esp3dSettingType::ip) {
      esp_err_t err;
      std::shared_ptr<nvs::NVSHandle> handle =
          nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
      if (err != ESP_OK) {
        esp3d_log_e("Failling accessing NVS");
      } else {
        std::string key = "p_" + std::to_string((uint)query->index);
        if (handle->set_item(key.c_str(), value) == ESP_OK) {
          return (handle->commit() == ESP_OK);
        }
      }
    }
  }
  return false;
}

bool ESP3dSettings::writeIPString(Esp3dSettingIndex index,
                                  const char* byte_buffer) {
  uint32_t ipInt = StringtoIPUInt32(byte_buffer);
  std::string ipStr = IPUInt32toString(ipInt);
  esp3d_log("write setting %d : %s to %ld to %s", static_cast<uint8_t>(index),
            byte_buffer, ipInt, ipStr.c_str());

  return writeUint32(index, StringtoIPUInt32(byte_buffer));
}

bool ESP3dSettings::writeString(Esp3dSettingIndex index,
                                const char* byte_buffer) {
  esp3d_log("write setting %d : (%d bytes) %s ", static_cast<uint16_t>(index),
            strlen(byte_buffer), byte_buffer);
  const Esp3dSettingDescription* query = getSettingPtr(index);
  if (query) {
    if (query->type == Esp3dSettingType::string_t &&
        strlen(byte_buffer) <= query->size) {
      esp_err_t err;
      std::shared_ptr<nvs::NVSHandle> handle =
          nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
      if (err != ESP_OK) {
        esp3d_log_e("Failling accessing NVS");
      } else {
        std::string key = "p_" + std::to_string((uint)query->index);
        if (handle->set_string(key.c_str(), byte_buffer) == ESP_OK) {
          esp3d_log("Write success");
          return (handle->commit() == ESP_OK);
        } else {
          esp3d_log_e("Write failed");
        }
      }
    } else {
      esp3d_log_e("Incorrect valued");
    }
  } else {
    esp3d_log_e("Unknow setting");
  }
  return false;
}

const char* ESP3dSettings::IPUInt32toString(uint32_t ip_int) {
  ip_addr_t tmpip;
  tmpip.type = IPADDR_TYPE_V4;
  // convert uint32_t to ip_addr_t
  ip_addr_set_ip4_u32_val(tmpip, ip_int);
  // convert ip_addr_t to_string
  return ip4addr_ntoa(&tmpip.u_addr.ip4);
}

uint32_t ESP3dSettings::StringtoIPUInt32(const char* s) {
  ip_addr_t tmpip;
  esp3d_log("convert %s", s);
  tmpip.type = IPADDR_TYPE_V4;
  // convert string to ip_addr_t
  ip4addr_aton(s, &tmpip.u_addr.ip4);
  // convert ip_addr_t to uint32_t
  return ip4_addr_get_u32(&tmpip.u_addr.ip4);
}

const Esp3dSettingDescription* ESP3dSettings::getSettingPtr(
    const Esp3dSettingIndex index) {
  for (uint16_t i = 0; i < sizeof(ESP3dSettingsData); i++) {
    if (ESP3dSettingsData[i].index == index) {
      return &ESP3dSettingsData[i];
    }
  }
  return nullptr;
}