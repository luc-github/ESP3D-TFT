/*
  esp3d_update_service
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

#include "esp3d_update_service.h"

#include <cstdlib>

#include "esp3d_client.h"
#include "esp3d_log.h"
#include "esp3d_hal.h"
#include "esp3d_string.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#include "sd_def.h"
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
#include "time/esp3d_time_service.h"
#endif  // ESP3D_TIMESTAMP_FEATURE

#include "network/esp3d_network.h"
#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif  // ESP3D_NOTIFICATIONS_FEATURE

#include "config_file/esp3d_config_file.h"

#define CONFIG_FILE "/esp3dcnf.ini"
#define CONFIG_FILE_OK "/esp3dcnf.ok"
#define FW_FILE "/esp3dfw.bin"
#define FW_FILE_OK "/esp3dfw.ok"
// #define FS_FILE "/esp3dfs.bin"
#define CHUNK_BUFFER_SIZE 1024

ESP3DUpdateService esp3dUpdateService;

const char *protectedkeys[] = {"NOTIF_TOKEN1",   "NOTIF_TOKEN2",
                               "AP_Password",    "STA_Password",
                               "ADMIN_PASSWORD", "USER_PASSWORD"};

// Network
// String values
const char *NetstringKeysVal[] = {"hostname",
#if ESP3D_WIFI_FEATURE
                                  "STA_SSID", "STA_Password", "AP_SSID",
                                  "AP_Password"
#endif  // ESP3D_WIFI_FEATURE
};

const ESP3DSettingIndex NetstringKeysPos[] = {
    ESP3DSettingIndex::esp3d_hostname,
#if ESP3D_WIFI_FEATURE
    ESP3DSettingIndex::esp3d_sta_ssid, ESP3DSettingIndex::esp3d_sta_password,
    ESP3DSettingIndex::esp3d_ap_ssid, ESP3DSettingIndex::esp3d_ap_password
#endif  // ESP3D_WIFI_FEATURE
};

// IP values
const char *IPKeysVal[] = {
#if ESP3D_WIFI_FEATURE
    "STA_IP", "STA_GW", "STA_MSK", "STA_DNS", "AP_IP"
#endif  // ESP3D_WIFI_FEATURE
};

const ESP3DSettingIndex IPKeysPos[] = {
#if ESP3D_WIFI_FEATURE
    ESP3DSettingIndex::esp3d_sta_ip_static,
    ESP3DSettingIndex::esp3d_sta_gw_static,
    ESP3DSettingIndex::esp3d_sta_mask_static,
    ESP3DSettingIndex::esp3d_sta_dns_static,
    ESP3DSettingIndex::esp3d_ap_ip_static
#endif  // ESP3D_WIFI_FEATURE
};
// Bytes values
const char *NetbyteKeysVal[] = {
#if ESP3D_WIFI_FEATURE
    "AP_channel"
#endif  // ESP3D_WIFI_FEATURE
};

const ESP3DSettingIndex NetbyteKeysPos[] = {
#if ESP3D_WIFI_FEATURE
    ESP3DSettingIndex::esp3d_ap_channel
#endif  // ESP3D_WIFI_FEATURE
};

// Services
// String values
const char *ServstringKeysVal[] = {
#if ESP3D_TIMESTAMP_FEATURE
    "Time_server1",   "Time_server2",  "Time_server3",        "Time_zone",
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    "ADMIN_PASSWORD", "USER_PASSWORD",
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    "NOTIF_TOKEN1",   "NOTIF_TOKEN2",  "NOTIF_TOKEN_Settings"
#endif  // ESP3D_NOTIFICATIONS_FEATURE
};

const ESP3DSettingIndex ServstringKeysPos[] = {
#if ESP3D_TIMESTAMP_FEATURE
    ESP3DSettingIndex::esp3d_time_server1,
    ESP3DSettingIndex::esp3d_time_server2,
    ESP3DSettingIndex::esp3d_time_server3,
    ESP3DSettingIndex::esp3d_timezone,
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    ESP3DSettingIndex::esp3d_admin_password,
    ESP3DSettingIndex::esp3d_user_password,
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    ESP3DSettingIndex::esp3d_notification_token_1,
    ESP3DSettingIndex::esp3d_notification_token_2,
    ESP3DSettingIndex::esp3d_notification_token_setting
#endif  // ESP3D_NOTIFICATIONS_FEATURE
};

// Integer values
const char *ServintKeysVal[] = {
#if ESP3D_HTTP_FEATURE
    "HTTP_Port",
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
    "TELNET_Port",
#endif  // ESP3D_TELNET_FEATURE

    //    "SENSOR_INTERVAL",
    //    "WebSocket_Port",
    //    "WebDav_Port",
    //    "FTP_Control_Port",
    //    "FTP_Active_Port ",
    //    "FTP_Passive_Port"
};

const ESP3DSettingIndex ServintKeysPos[] = {
#if ESP3D_HTTP_FEATURE
    ESP3DSettingIndex::esp3d_http_port,
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
    ESP3DSettingIndex::esp3d_socket_port,
#endif  // ESP3D_TELNET_FEATURE

    //    ESP_SENSOR_INTERVAL,
    //    esp3d_ws_port,
    //    ESP_WEBDAV_PORT,
    //    ESP_FTP_CTRL_PORT,
    //    ESP_FTP_DATA_ACTIVE_PORT,
    //    ESP_FTP_DATA_PASSIVE_PORT
};

// Boolean values
const char *ServboolKeysVal[] = {
#if ESP3D_HTTP_FEATURE
    "HTTP_active",
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    "AUTONOTIFICATION",
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_TELNET_FEATURE
    "TELNET_active",
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
    "WebDav_active",
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    "WebSocket_active",
#endif  // ESP3D_WS_SERVICE_FEATURE
//"WebDav_active",
#if ESP3D_SD_CARD_FEATURE
    "CHECK_FOR_UPDATE",
#endif  // ESP3D_SD_CARD_FEATURE

//"Active_buzzer",
#if ESP3D_TIMESTAMP_FEATURE
    "Active_Internet_time",
#endif  // ESP3D_TIMESTAMP_FEATURE
    "Radio_enabled"};

const ESP3DSettingIndex ServboolKeysPos[] = {
#if ESP3D_HTTP_FEATURE
    ESP3DSettingIndex::esp3d_http_on,
#endif  // ESP3D_HTTP_FEATURE

#if ESP3D_NOTIFICATIONS_FEATURE
    ESP3DSettingIndex::esp3d_auto_notification,
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_TELNET_FEATURE
    ESP3DSettingIndex::esp3d_socket_on,
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
    ESP3DSettingIndex::esp3d_webdav_on,
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    ESP3DSettingIndex::esp3d_ws_on,
#endif  // ESP3D_WS_SERVICE_FEATURE
// ESP_WEBDAV_ON,
// ESP_TIME_IS_DST,
#if ESP3D_SD_CARD_FEATURE
    ESP3DSettingIndex::esp3d_check_update_on_sd,
#endif  // ESP3D_SD_CARD_FEATURE
        // ESP_BUZZER,
#if ESP3D_TIMESTAMP_FEATURE
    ESP3DSettingIndex::esp3d_use_internet_time,
#endif  // ESP3D_TIMESTAMP_FEATURE
    ESP3DSettingIndex::esp3d_radio_boot_mode};

// Byte values
const char *ServbyteKeysVal[] = {
//"Time_zone",
#if ESP3D_AUTHENTICATION_FEATURE
    "Sesion_timeout",
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_IS_SPI
    "SD_SPEED"
#endif  // ESP3D_SD_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE

    //"Time_DST"
};

const ESP3DSettingIndex ServbyteKeysPos[] = {
// ESP_TIMEZONE,
#if ESP3D_AUTHENTICATION_FEATURE
    ESP3DSettingIndex::esp3d_session_timeout,
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_IS_SPI
    ESP3DSettingIndex::esp3d_spi_divider
#endif  // ESP3D_SD_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE

    // ESP_TIME_DST
};

// System
// Integer values
const char *SysintKeysVal[] = {"Baud_rate",
#if ESP3D_USB_SERIAL_FEATURE
                               "USB_Serial_Baud_rate"
#endif  // #if ESP3D_USB_SERIAL_FEATURE
        //"Boot_delay"
};

const ESP3DSettingIndex SysintKeysPos[] = {
    ESP3DSettingIndex::esp3d_baud_rate,
#if ESP3D_USB_SERIAL_FEATURE
    ESP3DSettingIndex::esp3d_usb_serial_baud_rate
#endif  // #if ESP3D_USB_SERIAL_FEATURE
        // ESP_BOOT_DELAY
};
// Boolean values
const char *SysboolKeysVal[] = {
    //"Active_Serial ",
    //"Active_WebSocket",
    //"Active_Telnet",
    //"Active_BT",
    //"Boot_verbose",
    //"Secure_serial"
};

const ESP3DSettingIndex SysboolKeysPos[] = {
    // ESP_SERIAL_FLAG,
    // ESP_WEBSOCKET_FLAG,
    // ESP_TELNET_FLAG,
    // ESP_BT_FLAG,
    // ESP_VERBOSE_BOOT,
    // ESP_SECURE_SERIAL
};

ESP3DUpdateService::ESP3DUpdateService() {}

ESP3DUpdateService::~ESP3DUpdateService() {}

bool ESP3DUpdateService::canUpdate() {
  const esp_partition_t *running = esp_ota_get_running_partition();
  const esp_partition_t *update_partition =
      esp_ota_get_next_update_partition(NULL);
  if (!running) {
    esp3d_log_e("Cannot get running partition");
    return false;
  }
  esp_app_desc_t running_app_info;
  esp3d_log("Running partition subtype %d at offset 0x%lx", running->subtype,
            running->address);
  if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
    esp3d_log("Running firmware version: %s", running_app_info.version);
  }
  if (!update_partition) {
    esp3d_log_e("Cannot get update partition");
    return false;
  }
  esp3d_log("Update partition subtype %d at offset 0x%lx",
            update_partition->subtype, update_partition->address);
  return true;
}

size_t ESP3DUpdateService::maxUpdateSize() {
  size_t max_size = 0;
  const esp_partition_t *update_partition =
      esp_ota_get_next_update_partition(NULL);
  if (!update_partition) {
    esp3d_log_e("Cannot get update partition");
  } else {
    max_size = update_partition->size;
  }
  return max_size;
}

bool ESP3DUpdateService::begin() {
  esp3d_log("Starting Update Service");
  bool restart = false;
  ESP3DState setting_check_update = ESP3DState::off;
#if ESP3D_SD_CARD_FEATURE
  setting_check_update = (ESP3DState)esp3dTftsettings.readByte(
      ESP3DSettingIndex::esp3d_check_update_on_sd);
#endif  // ESP3D_SD_CARD_FEATURE

  if (setting_check_update == ESP3DState::off || !canUpdate()) {
    esp3d_log("Update Service disabled");
    return true;
  }
#if ESP3D_SD_CARD_FEATURE
  if (sd.accessFS()) {
    if (sd.exists(FW_FILE)) {
      restart = updateFW();
      if (restart) {
        std::string filename = FW_FILE_OK;
        uint8_t n = 1;
        // if FW_FILE_OK already exists,  rename it to FW_FILE_OKXX
        if (sd.exists(filename.c_str())) {
          // find proper name
          while (sd.exists(filename.c_str())) {
            filename = FW_FILE_OK + std::to_string(n++);
          }
          // rename FW_FILE_OK to FW_FILE_OKXX
          if (!sd.rename(FW_FILE_OK, filename.c_str())) {
            esp3d_log_e("Failed to rename %s", FW_FILE_OK);
            // to avoid dead loop
            restart = false;
          }
        }
        // rename FW_FILE to FW_FILE_OK
        if (!sd.rename(FW_FILE, FW_FILE_OK)) {
          esp3d_log_e("Failed to rename %s", FW_FILE);
          // to avoid dead loop
          restart = false;
        }
      }
    } else {
      esp3d_log("No Fw update on SD");
      if (sd.exists(CONFIG_FILE)) {
        restart = updateConfig();
      }
    }
    sd.releaseFS();
  } else {
    esp3d_log("SD unavailable for update");
  }
#endif  // ESP3D_SD_CARD_FEATURE
  if (restart) {
    esp3d_log("Restarting  firmware");
    esp3d_hal::wait(1000);
    esp_restart();
    while (1) {
      esp3d_hal::wait(10);
    }
  }
  return true;
}

bool ESP3DUpdateService::updateConfig() {
  bool res = false;
  ESP3DConfigFile updateConfiguration(
      ESP3D_SD_FS_HEADER CONFIG_FILE, esp3dUpdateService.processingFileFunction,
      ESP3D_SD_FS_HEADER CONFIG_FILE_OK, protectedkeys);
  if (updateConfiguration.processFile()) {
    esp3d_log("Processing ini file done");
    if (updateConfiguration.revokeFile()) {
      esp3d_log("Revoking ini file done");
      res = true;
    } else {
      esp3d_log_e("Revoking ini file failed");
    }
  } else {
    esp3d_log_e("Processing ini file failed");
  }
  return res;
}
#if ESP3D_SD_CARD_FEATURE
bool ESP3DUpdateService::updateFW() {
  bool isSuccess = true;
  char chunk[CHUNK_BUFFER_SIZE];
  esp_ota_handle_t update_handle = 0;
  const esp_partition_t *update_partition = NULL;
  esp3d_log("Updating firmware");
  struct stat entry_stat;
  size_t totalSize = 0;
  if (sd.stat(FW_FILE, &entry_stat) == -1) {
    esp3d_log_e("Failed to stat : %s", FW_FILE);
    return false;
  }
  FILE *fwFd = sd.open(FW_FILE, "r");
  if (!fwFd) {
    esp3d_log_e("Failed to open on sd : %s", FW_FILE);
    return false;
  }
  update_partition = esp_ota_get_next_update_partition(NULL);
  if (!update_partition) {
    esp3d_log_e("Error accessing flash filesystem");
    isSuccess = false;
  }
  if (isSuccess) {
    esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES,
                                  &update_handle);
    if (err != ESP_OK) {
      esp3d_log_e("esp_ota_begin failed (%s)", esp_err_to_name(err));
      isSuccess = false;
    }
  }
  if (isSuccess) {
    size_t chunksize;
#if ESP3D_TFT_LOG
    uint8_t progress = 0;
#endif
    do {
      chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, fwFd);
      totalSize += chunksize;
      if (esp_ota_write(update_handle, (const void *)chunk, chunksize) !=
          ESP_OK) {
        esp3d_log_e("Error cannot writing data on update partition");
        isSuccess = false;
      }
#if ESP3D_TFT_LOG
      if (progress != 100 * totalSize / entry_stat.st_size) {
        progress = 100 * totalSize / entry_stat.st_size;
        esp3d_log("Update %d %% %d / %ld", progress, totalSize,
                  entry_stat.st_size);
      }
#endif
    } while (chunksize != 0 && isSuccess);
  }
  sd.close(fwFd);
  if (isSuccess) {
    // check size
    if (totalSize != entry_stat.st_size) {
      esp3d_log_e("Failed to read firmware full data");
      isSuccess = false;
    }
  }
  if (isSuccess) {
    esp_err_t err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
      esp3d_log_e("Error cannot end update(%s)", esp_err_to_name(err));
      isSuccess = false;
    }
    update_handle = 0;
  }
  if (isSuccess) {
    esp_err_t err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
      esp3d_log_e("esp_ota_set_boot_partition failed (%s)!",
                  esp_err_to_name(err));
      isSuccess = false;
    }
  }
  if (update_handle && !isSuccess) {
    esp_ota_abort(update_handle);
    update_handle = 0;
  }
  return isSuccess;
}
#endif  // ESP3D_SD_CARD_FEATURE
void ESP3DUpdateService::handle() {}

void ESP3DUpdateService::end() { esp3d_log("Stop Update Service"); }

bool ESP3DUpdateService::processString(const char **keysval,
                                       const ESP3DSettingIndex *keypos,
                                       const size_t size, const char *key,
                                       const char *value, char &T,
                                       ESP3DSettingIndex &P) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      // if it is a previouly saved scrambled password ignore it
      if (strcasecmp(value, "********") != 0) {
        T = 'S';
        P = keypos[i];
        esp3d_log("It is a string key");
        return true;
      }
    }
  }
  esp3d_log("Not a string key");
  return false;
}

bool ESP3DUpdateService::processInt(const char **keysval,
                                    const ESP3DSettingIndex *keypos,
                                    const size_t size, const char *key,
                                    const char *value, char &T,
                                    ESP3DSettingIndex &P, uint32_t &v) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      T = 'I';
      P = keypos[i];
      v = atoi(value);
      esp3d_log("It is an integer key");
      return true;
    }
  }
  esp3d_log("Not an integer key");
  return false;
}

bool ESP3DUpdateService::processBool(const char **keysval,
                                     const ESP3DSettingIndex *keypos,
                                     const size_t size, const char *key,
                                     const char *value, char &T,
                                     ESP3DSettingIndex &P, uint8_t &b) {
  for (uint i = 0; i < size; i++) {
    if (strcasecmp(keysval[i], key) == 0) {
      T = 'B';
      P = keypos[i];
      if ((strcasecmp("yes", value) == 0) || (strcasecmp("on", value) == 0) ||
          (strcasecmp("true", value) == 0) || (strcasecmp("1", value) == 0)) {
        b = 1;
      } else if ((strcasecmp("no", value) == 0) ||
                 (strcasecmp("off", value) == 0) ||
                 (strcasecmp("false", value) == 0) ||
                 (strcasecmp("0", value) == 0)) {
        b = 0;
      } else {
        esp3d_log("Not a valid boolean key");
        return false;
      }
      esp3d_log("Not a boolean key");
      return true;
    }
  }
  esp3d_log("Not a boolean key");
  return false;
}

bool ESP3DUpdateService::processingFileFunction(const char *section,
                                                const char *key,
                                                const char *value) {
  esp3d_log("Processing Section: %s, Key: %s, Value: %s", section, key, value);
  bool res = true;
  char T = '\0';
  ESP3DSettingIndex P = ESP3DSettingIndex::unknown_index;
  uint32_t v = 0;
  uint8_t b = 0;
  bool done = false;
  // network / services / system sections
  if (strcasecmp("network", section) == 0) {
    if (!done) {
      done = processString(NetstringKeysVal, NetstringKeysPos,
                           sizeof(NetstringKeysVal) / sizeof(char *), key,
                           value, T, P);
    }
    if (!done) {
      done =
          processString(IPKeysVal, IPKeysPos,
                        sizeof(IPKeysVal) / sizeof(char *), key, value, T, P);
      if (done) {
        T = 'A';
      }
    }
    if (!done) {
      done = processInt(NetbyteKeysVal, NetbyteKeysPos,
                        sizeof(NetbyteKeysVal) / sizeof(char *), key, value, T,
                        P, v);
      if (done) {
        T = 'B';
        b = v;
      }
    }
    // Radio mode BT, STA, AP, OFF
    if (!done) {
      if (strcasecmp("radio_mode", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_radio_mode;
        done = true;
        if (strcasecmp("BT", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::bluetooth_serial);
        } else
#if ESP3D_WIFI_FEATURE
            if (strcasecmp("STA", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::wifi_sta);
        } else if (strcasecmp("AP", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::wifi_ap);
        } else if (strcasecmp("SETUP", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::wifi_ap_config);
        } else
#endif  // ESP3D_WIFI_FEATURE
          if (strcasecmp("OFF", value) == 0) {
            b = static_cast<uint8_t>(ESP3DRadioMode::off);
          } else {
            P = ESP3DSettingIndex::unknown_index;  // invalid value
          }
      }
    }
#if ESP3D_WIFI_FEATURE
    // STA fallback mode BT, WIFI-AP, OFF
    if (!done) {
      if (strcasecmp("sta_fallback", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_fallback_mode;
        done = true;
        if (strcasecmp("BT", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::bluetooth_serial);
        } else if (strcasecmp("WIFI-SETUP", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::wifi_ap_config);
        } else if (strcasecmp("OFF", value) == 0) {
          b = static_cast<uint8_t>(ESP3DRadioMode::off);
        } else {
          P = ESP3DSettingIndex::unknown_index;  // invalid value
        }
      }
    }

    // STA IP Mode DHCP / STATIC
    if (!done) {
      if (strcasecmp("STA_IP_mode", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_sta_ip_mode;
        done = true;
        if (strcasecmp("DHCP", value) == 0) {
          b = static_cast<uint8_t>(ESP3DIpMode::dhcp);
        } else if (strcasecmp("STATIC", key) == 0) {
          b = static_cast<uint8_t>(ESP3DIpMode::staticIp);
        } else {
          P = ESP3DSettingIndex::unknown_index;  // invalid value
        }
      }
    }
#endif  // ESP3D_WIFI_FEATURE
  } else if (strcasecmp("services", section) == 0) {
    if (!done) {
      done = processString(ServstringKeysVal, ServstringKeysPos,
                           sizeof(ServstringKeysVal) / sizeof(char *), key,
                           value, T, P);
    }
    if (!done) {
      done = processInt(ServintKeysVal, ServintKeysPos,
                        sizeof(ServintKeysVal) / sizeof(char *), key, value, T,
                        P, v);
    }
    if (!done) {
      done = processBool(ServboolKeysVal, ServboolKeysPos,
                         sizeof(ServboolKeysVal) / sizeof(char *), key, value,
                         T, P, b);
    }
    if (!done) {
      done = processInt(ServbyteKeysVal, ServbyteKeysPos,
                        sizeof(ServbyteKeysVal) / sizeof(char *), key, value, T,
                        P, v);
      if (done) {
        T = 'B';
        b = v;
      }
    }
    // Notification type None / PushOver / Line / Email / Telegram / IFTTT
    if (!done) {
#if ESP3D_NOTIFICATIONS_FEATURE
      if (strcasecmp("NOTIF_TYPE", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_notification_type;
        done = true;
        if (strcasecmp("None", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::none);
        } else if (strcasecmp("PushOver", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::pushover);
        } else if (strcasecmp("Line", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::line);
        } else if (strcasecmp("Email", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::email);
        } else if (strcasecmp("Telegram", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::telegram);
        } else if (strcasecmp("IFTTT", value) == 0) {
          b = static_cast<uint8_t>(ESP3DNotificationType::ifttt);
        } else {
          P = ESP3DSettingIndex::unknown_index;  // invalid value
        }
      }
#endif  // ESP3D_NOTIFICATIONS_FEATURE
    }
  } else if (strcasecmp("system", section) == 0) {
    if (!done) {
      done = processInt(SysintKeysVal, SysintKeysPos,
                        sizeof(SysintKeysVal) / sizeof(char *), key, value, T,
                        P, v);
    }
    if (!done) {
      done = processBool(SysboolKeysVal, SysboolKeysPos,
                         sizeof(SysboolKeysVal) / sizeof(char *), key, value, T,
                         P, b);
    }
    // Target Firmware None / Marlin / Repetier / MarlinKimbra / Smoothieware /
    // GRBL
    if (!done) {
      if (strcasecmp("TargetFW", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_target_firmware;
        done = true;
        if (strcasecmp("None", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::unknown);
        } else if (strcasecmp("MARLIN", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::marlin);
        } else if (strcasecmp("GRBLHAL", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::grblhal);
        } else if (strcasecmp("GRBL", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::grbl);
        } else if (strcasecmp("REPETIER", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::repetier);
        } else if (strcasecmp("SMOOTHIEWARE", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::smoothieware);
        } else if (strcasecmp("HP_GL", value) == 0) {
          b = static_cast<uint8_t>(ESP3DTargetFirmware::hp_gl);
        } else {
          P = ESP3DSettingIndex::unknown_index;  // invalid value
        }
      }
    }
    // Output: SERIAL / USB
    if (!done) {
      if (strcasecmp("output", key) == 0) {
        T = 'B';
        P = ESP3DSettingIndex::esp3d_output_client;
        done = true;
        if (strcasecmp("USB", value) == 0) {
          b = static_cast<uint8_t>(ESP3DClientType::usb_serial);
        } else if (strcasecmp("SERIAL", value) == 0) {
          b = static_cast<uint8_t>(ESP3DClientType::serial);
        } else {
          P = ESP3DSettingIndex::unknown_index;  // invalid value
        }
      }
    }
  }

  // now we save - handle saving status
  // if setting is not recognized it is not a problem
  // but if save is fail - that is a problem - so report it
  if (P != ESP3DSettingIndex::unknown_index) {
    switch (T) {
      case 'S':
        if (esp3dTftsettings.isValidStringSetting(value, P)) {
          res = esp3dTftsettings.writeString(P, value);
        } else {
          esp3d_log_e("Value \"%s\" is invalid", value);
        }
        break;
      case 'B':
        if (esp3dTftsettings.isValidByteSetting(b, P)) {
          res = esp3dTftsettings.writeByte(P, b);
        } else {
          esp3d_log_e("Value \"%d\" is invalid", b);
        }
        break;
      case 'I':
        if (esp3dTftsettings.isValidIntegerSetting(v, P)) {
          res = esp3dTftsettings.writeUint32(P, v);
        } else {
          esp3d_log_e("Value \"%ld\" is invalid", v);
        }
        break;
      case 'A':
        if (esp3dTftsettings.isValidIPStringSetting(value, P)) {
          res = esp3dTftsettings.writeIPString(P, value);
        } else {
          esp3d_log_e("Value \"%s\" is invalid", value);
        }
        break;
      default:
        esp3d_log_e("Unknown flag");
    }
  } else {
    esp3d_log_e("Unknown value");
  }
  return res;
}