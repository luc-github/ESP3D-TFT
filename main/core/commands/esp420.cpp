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

#include <stdio.h>

#include <cstring>
#include <string>

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_wifi_ap_get_sta_list.h"
#include "filesystem/esp3d_flash.h"
#include "gcode_host/esp3d_gcode_host_service.h"
#include "network/esp3d_network.h"
#include "rom/ets_sys.h"
#include "sdkconfig.h"
#include "spi_flash_mmap.h"
#include "translations/esp3d_translation_service.h"

#if CONFIG_SPIRAM
#include "esp_psram.h"
#endif  // CONFIG_SPIRAM

#if ESP3D_HTTP_FEATURE
#include "http/esp3d_http_service.h"
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_UPDATE_FEATURE
#include "update/esp3d_update_service.h"
#endif  // ESP3D_UPDATE_FEATURE
#if ESP3D_MDNS_FEATURE
#include "mdns/esp3d_mdns.h"
#endif  // ESP3D_MDNS_FEATURE
#if ESP3D_SSDP_FEATURE
#include "ssdp/esp3d_ssdp.h"
#endif  // ESP3D_SSDP_FEATURE
#if ESP3D_TELNET_FEATURE
#include "socket_server/esp3d_socket_server.h"
#endif  // ESP3D_TELNET_FEATURE
#include "websocket/esp3d_ws_service.h"
#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // #if ESP3D_USB_SERIAL_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
#include "time/esp3d_time_service.h"
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_CAMERA_FEATURE
#include "camera/camera.h"
#endif  // ESP3D_CAMERA_FEATURE

#define COMMAND_ID 420

// Get ESP current status
// output is JSON or plain text according parameter
//[ESP420]json=<no>
void ESP3DCommands::ESP420(int cmd_params_pos, ESP3DMessage *msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool addPreTag = hasTag(msg, cmd_params_pos, "addPreTag");
  std::string tmpstr;
  multi_heap_info_t info;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (json) {
    tmpstr = "{\"cmd\":\"420\",\"status\":\"ok\",\"data\":[";

  } else {
    if (addPreTag) {
      tmpstr = "<pre>\n";
    } else {
      tmpstr = "Configuration:\n";
    }
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  // Screen
  if (!dispatchIdValue(json, "Screen", TFT_TARGET, target, requestId, true)) {
    return;
  }

  // ESP3D-TFT-VERSION
  if (!dispatchIdValue(json, "FW ver", ESP3D_TFT_VERSION, target, requestId)) {
    return;
  }

  // FW Arch
  if (!dispatchIdValue(json, "FW arch", CONFIG_IDF_TARGET, target, requestId)) {
    return;
  }

  // SDK Version
  if (!dispatchIdValue(json, "SDK", IDF_VER, target, requestId)) {
    return;
  }

  // CPU Freq
  tmpstr = std::to_string(ets_get_cpu_frequency()) + "MHz";
  if (!dispatchIdValue(json, "CPU Freq", tmpstr.c_str(), target, requestId)) {
    return;
  }

  // Free memory
  tmpstr = esp3d_string::formatBytes(esp_get_minimum_free_heap_size());
  if (!dispatchIdValue(json, "free mem", tmpstr.c_str(), target, requestId)) {
    return;
  }

  heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);
  tmpstr = esp3d_string::formatBytes(info.total_allocated_bytes);
  tmpstr += " / ";
  tmpstr += esp3d_string::formatBytes(info.total_free_bytes +
                                      info.total_allocated_bytes);
  if (!dispatchIdValue(json, "heap usage", tmpstr.c_str(), target, requestId)) {
    return;
  }

#if CONFIG_SPIRAM

  tmpstr = esp3d_string::formatBytes(esp_psram_get_size());
  if (!dispatchIdValue(json, "Total psram mem", tmpstr.c_str(), target,
                       requestId)) {
    return;
  }

  heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
  tmpstr = esp3d_string::formatBytes(info.total_allocated_bytes);
  tmpstr += " / ";
  tmpstr += esp3d_string::formatBytes(info.total_free_bytes +
                                      info.total_allocated_bytes);
  if (!dispatchIdValue(json, "psram heap usage", tmpstr.c_str(), target,
                       requestId)) {
    return;
  }
#endif  // CONFIG_SPIRAM

  // Flash size
  uint32_t flash_size;
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    esp3d_log_e("Get flash size failed");
    return;
  }

  tmpstr = esp3d_string::formatBytes(flash_size);
  if (!dispatchIdValue(json, "flash size", tmpstr.c_str(), target, requestId)) {
    return;
  }
#if ESP3D_UPDATE_FEATURE
  // Update max
  tmpstr = esp3d_string::formatBytes(esp3dUpdateService.maxUpdateSize());
  if (!dispatchIdValue(json, "size for update", tmpstr.c_str(), target,
                       requestId)) {
    return;
  }
#if ESP3D_SD_CARD_FEATURE
  // SD updater
  ESP3DState statesetting = (ESP3DState)esp3dTftsettings.readByte(
      ESP3DSettingIndex::esp3d_check_update_on_sd);
  if (statesetting == ESP3DState::off) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON";
  }
  if (!dispatchIdValue(json, "SD updater", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // ESP3D_SD_CARD_FEATURE

#else
  if (!dispatchIdValue(json, "FW Update", "OFF", target, requestId)) {
    return;
  }
#endif  // ESP3D_UPDATE_FEATURE
  // Flash FS
  tmpstr = flashFs.getFileSystemName();
  if (!dispatchIdValue(json, "flash fs", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // FileSystem
  size_t totalBytes = 0;
  size_t usedBytes = 0;
  flashFs.getSpaceInfo(&totalBytes, &usedBytes);
  tmpstr = esp3d_string::formatBytes(usedBytes);
  tmpstr += " / ";
  tmpstr += esp3d_string::formatBytes(totalBytes);
  if (!dispatchIdValue(json, "FS usage", tmpstr.c_str(), target, requestId)) {
    return;
  }

  if (esp3dCommands.getOutputClient() == ESP3DClientType::serial) {
    if (!dispatchIdValue(json, "output", "serial port", target, requestId)) {
      return;
    }
    // Serial BaudRate
    uint32_t baud =
        esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_baud_rate);
    tmpstr = std::to_string(baud);
    if (!dispatchIdValue(json, "baud", tmpstr.c_str(), target, requestId)) {
      return;
    }
  }
#if ESP3D_USB_SERIAL_FEATURE
  else if (esp3dCommands.getOutputClient() == ESP3DClientType::usb_serial) {
    if (!dispatchIdValue(json, "output", "usb port", target, requestId)) {
      return;
    }
    uint32_t baud = esp3dTftsettings.readUint32(
        ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
    tmpstr = std::to_string(baud);
    if (!dispatchIdValue(json, "baud", tmpstr.c_str(), target, requestId)) {
      return;
    }
    if (!dispatchIdValue(json, "clients",
                         usbSerialClient.isConnected() ? "1" : "0", target,
                         requestId)) {
      return;
    }
  }
#endif  // #if ESP3D_USB_SERIAL
  // Streaming buffer of commands and files

  size_t stream_size = gcodeHostService.getScriptsListSize();
  float perct = 100.0 * stream_size / ESP3D_MAX_STREAM_SIZE;
  tmpstr = esp3d_string::set_precision(std::to_string(perct).c_str(), 1);
  tmpstr += "% (";
  tmpstr += std::to_string(stream_size);
  tmpstr += "/";
  tmpstr += std::to_string(ESP3D_MAX_STREAM_SIZE);
  tmpstr += ")";
  stream_size = gcodeHostService.getStreamsListSize();
  perct = 100.0 * stream_size / ESP3D_MAX_STREAM_SIZE;
  tmpstr += " - ";
  tmpstr += esp3d_string::set_precision(std::to_string(perct).c_str(), 1);
  tmpstr += "% (";
  tmpstr += std::to_string(stream_size);
  tmpstr += "/";
  tmpstr += std::to_string(ESP3D_MAX_STREAM_SIZE);
  tmpstr += ")";
  if (!dispatchIdValue(json, "Streaming buffer", tmpstr.c_str(), target,
                       requestId)) {
    return;
  }

  // wifi
  if (esp3dNetwork.getMode() == ESP3DRadioMode::off ||
      esp3dNetwork.getMode() == ESP3DRadioMode::bluetooth_serial) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON";
  }
  if (!dispatchIdValue(json, "wifi", tmpstr.c_str(), target, requestId)) {
    return;
  }

  // hostname
  const ESP3DSettingDescription *settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_hostname);
  if (settingPtr) {
    char out_str[(settingPtr->size) + 1] = {0};
    tmpstr = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_hostname,
                                         out_str, settingPtr->size);
  } else {
    tmpstr = "Error!!";
  }
  if (!dispatchIdValue(json, "hostname", tmpstr.c_str(), target, requestId)) {
    return;
  }
#if ESP3D_WIFI_FEATURE
  // sta
  if (esp3dNetwork.getMode() == ESP3DRadioMode::wifi_sta) {
    tmpstr = "ON";
  } else {
    tmpstr = "OFF";
  }
  tmpstr += " (";
  tmpstr += esp3dNetwork.getSTAMac();
  tmpstr += ")";

  if (!dispatchIdValue(json, "sta", tmpstr.c_str(), target, requestId)) {
    return;
  }

  if (esp3dNetwork.getMode() == ESP3DRadioMode::wifi_sta) {
    wifi_ap_record_t ap;
    esp_err_t res = esp_wifi_sta_get_ap_info(&ap);
    if (res == ESP_OK) {
      // ssid
      tmpstr = (const char *)ap.ssid;
      if (!dispatchIdValue(json, "SSID", tmpstr.c_str(), target, requestId)) {
        return;
      }
      //%signal
      int32_t signal = esp3dNetwork.getSignal(ap.rssi, false);
      tmpstr = std::to_string(signal);
      tmpstr += "%";
      if (!dispatchIdValue(json, "signal", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr = std::to_string(ap.primary);
      if (!dispatchIdValue(json, "channel", tmpstr.c_str(), target,
                           requestId)) {
        return;
      }
    } else {
      if (!dispatchIdValue(json, "status", "not connected", target,
                           requestId)) {
        return;
      }
    }

    if (esp3dNetwork.useStaticIp()) {
      tmpstr = "static";
    } else {
      tmpstr = "dhcp";
    }
    if (!dispatchIdValue(json, "ip mode", tmpstr.c_str(), target, requestId)) {
      return;
    }
    ESP3DIpInfos ipInfo;
    if (esp3dNetwork.getLocalIp(&ipInfo)) {
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.ip));
      if (!dispatchIdValue(json, "ip", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.gw));
      if (!dispatchIdValue(json, "gw", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.netmask));
      if (!dispatchIdValue(json, "msk", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr =
          ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.dns_info.ip.u_addr.ip4));
      if (!dispatchIdValue(json, "DNS", tmpstr.c_str(), target, requestId)) {
        return;
      }
    }
  }
  // ap
  if (esp3dNetwork.getMode() == ESP3DRadioMode::wifi_ap ||
      esp3dNetwork.getMode() == ESP3DRadioMode::wifi_ap_config) {
    tmpstr = "ON";
  } else {
    tmpstr = "OFF";
  }

  tmpstr += " (";
  tmpstr += esp3dNetwork.getAPMac();
  tmpstr += ")";
  if (!dispatchIdValue(json, "ap", tmpstr.c_str(), target, requestId)) {
    return;
  }
  if (esp3dNetwork.getMode() == ESP3DRadioMode::wifi_ap_config) {
    if (!dispatchIdValue(json, "config", "ON", target, requestId)) {
      return;
    }
  }
  if (esp3dNetwork.getMode() == ESP3DRadioMode::wifi_ap ||
      esp3dNetwork.getMode() == ESP3DRadioMode::wifi_ap_config) {
    wifi_config_t wconfig;
    esp_err_t res = esp_wifi_get_config(WIFI_IF_AP, &wconfig);
    if (res == ESP_OK) {
      // ssid
      tmpstr = (const char *)wconfig.ap.ssid;
      if (!dispatchIdValue(json, "SSID", tmpstr.c_str(), target, requestId)) {
        return;
      }

      tmpstr = std::to_string(wconfig.ap.channel);
      if (!dispatchIdValue(json, "channel", tmpstr.c_str(), target,
                           requestId)) {
        return;
      }
    }
    ESP3DIpInfos ipInfo;
    if (esp3dNetwork.getLocalIp(&ipInfo)) {
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.ip));
      if (!dispatchIdValue(json, "ip", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.gw));
      if (!dispatchIdValue(json, "gw", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.ip_info.netmask));
      if (!dispatchIdValue(json, "msk", tmpstr.c_str(), target, requestId)) {
        return;
      }
      tmpstr =
          ip4addr_ntoa((const ip4_addr_t *)&(ipInfo.dns_info.ip.u_addr.ip4));
      if (!dispatchIdValue(json, "DNS", tmpstr.c_str(), target, requestId)) {
        return;
      }
    }
    wifi_sta_list_t sta_list;
    wifi_sta_mac_ip_list_t tcpip_sta_list;
    ESP_ERROR_CHECK(esp_wifi_ap_get_sta_list(&sta_list));
    if (sta_list.num > 0) {
      ESP_ERROR_CHECK(
          esp_wifi_ap_get_sta_list_with_ip(&sta_list, &tcpip_sta_list));
    }

    tmpstr = std::to_string(sta_list.num);

    if (!dispatchIdValue(json, "clients", tmpstr.c_str(), target, requestId)) {
      return;
    }
    for (int i = 0; i < sta_list.num; i++) {
      // Print the mac address of the connected station

      tmpstr = ip4addr_ntoa((const ip4_addr_t *)&(tcpip_sta_list.sta[i].ip));
      tmpstr += "(";
      tmpstr += esp3dNetwork.getMacAddress(sta_list.sta[i].mac);
      tmpstr += ")";
      std::string client = "# ";
      client += std::to_string(i + 1);
      if (!dispatchIdValue(json, client.c_str(), tmpstr.c_str(), target,
                           requestId)) {
        return;
      }
    }
  }
#endif  // ESP3D_WIFI_FEATURE
  // bt
  if (esp3dNetwork.getMode() == ESP3DRadioMode::bluetooth_serial) {
    tmpstr = "ON";
  } else {
    tmpstr = "OFF";
  }
  tmpstr += " (";
  tmpstr += esp3dNetwork.getBTMac();
  tmpstr += ")";
  if (!dispatchIdValue(json, "bt", tmpstr.c_str(), target, requestId)) {
    return;
  }

#if ESP3D_MDNS_FEATURE
  // mdsn service
  if (!esp3dmDNS.started()) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON";
  }

  if (!dispatchIdValue(json, "mDNS", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // ESP3D_MDNS_FEATURE

#if ESP3D_SSDP_FEATURE
  // ssdp service
  if (!esp3d_ssdp_service.started()) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON";
  }

  if (!dispatchIdValue(json, "ssdp", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // ESP3D_SSDP_FEATURE
#if ESP3D_TELNET_FEATURE
  // socket server
  if (!esp3dSocketServer.started()) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON (";
    tmpstr += std::to_string(esp3dSocketServer.port());
    tmpstr += ")";
  }

  if (!dispatchIdValue(json, "telnet", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // Socket connected clients
  tmpstr = std::to_string(esp3dSocketServer.clientsConnected());
  if (esp3dSocketServer.started()) {
    if (!dispatchIdValue(json, "clients", tmpstr.c_str(), target, requestId)) {
      return;
    }
    for (int i = 0; i < ESP3D_MAX_SOCKET_CLIENTS; i++) {
      // Print the mac address of the connected station
      if (esp3dSocketServer.getClientInfos(i)) {
        char addr_str[16];
        inet_ntoa_r(((struct sockaddr_in *)&(
                         esp3dSocketServer.getClientInfos(i)->source_addr))
                        ->sin_addr,
                    addr_str, sizeof(addr_str) - 1);
        tmpstr = addr_str;
        std::string client = "# ";
        client += std::to_string(i + 1);
        if (!dispatchIdValue(json, client.c_str(), tmpstr.c_str(), target,
                             requestId)) {
          return;
        }
      }
    }
  }
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_HTTP_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
  // WebDav service
  if (esp3dHttpService.started() && esp3dHttpService.webdavActive()) {
    tmpstr = "ON";
  } else {
    tmpstr = "OFF";
  }

  if (!dispatchIdValue(json, "WebDav", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
  // Web socket server
  if (!esp3dWsDataService.started()) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON";
  }

  if (!dispatchIdValue(json, "ws", tmpstr.c_str(), target, requestId)) {
    return;
  }

  if (esp3dWsDataService.started()) {
    // WebSocket connected clients
    tmpstr = std::to_string(esp3dWsDataService.clientsConnected());

    if (!dispatchIdValue(json, "clients", tmpstr.c_str(), target, requestId)) {
      return;
    }
    for (int i = 0; i < esp3dWsDataService.maxClients(); i++) {
      // Print the mac address of the connected station
      if (esp3dWsDataService.getClientInfos(i)) {
        char addr_str[16];
        inet_ntoa_r(((struct sockaddr_in *)&(
                         esp3dWsDataService.getClientInfos(i)->source_addr))
                        ->sin_addr,
                    addr_str, sizeof(addr_str) - 1);
        tmpstr = addr_str;
        std::string client = "# ";
        client += std::to_string(i + 1);
        if (!dispatchIdValue(json, client.c_str(), tmpstr.c_str(), target,
                             requestId)) {
          return;
        }
      }
    }
  }
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_CAMERA_FEATURE
  if (esp3d_camera.started()) {
    // camera name
    tmpstr = esp3d_camera.GetModelString();
    tmpstr += "(";
    tmpstr += std::to_string(esp3d_camera.GetModel());
    tmpstr += ")";

    if (!dispatchIdValue(json, "camera name", tmpstr.c_str(), target,
                         requestId)) {
      return;
    }
  } else {
    if (!dispatchIdValue(json, "camera name", "OFF", target, requestId)) {
      return;
    }
  }
#endif  // ESP3D_CAMERA_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
  // Notifications
  if (!esp3dNotificationsService.started() ||
      esp3dNotificationsService.getType() == ESP3DNotificationType::none) {
    tmpstr = "OFF";
  } else {
    tmpstr = "ON (";
    tmpstr += esp3dNotificationsService.getTypeString();
    tmpstr += ")";
  }

  if (!dispatchIdValue(json, "notification", tmpstr.c_str(), target,
                       requestId)) {
    return;
  }
#endif  // ESP3D_NOTIFICATIONS_FEATURE
        // UI language
  tmpstr = esp3dTranslationService.translate(ESP3DLabel::language);
  if (tmpstr != "???") {
    tmpstr += " (";
    tmpstr += esp3dTranslationService.getLanguageCode();
    tmpstr += ")";
  }
  if (!dispatchIdValue(json, "language", tmpstr.c_str(), target, requestId)) {
    return;
  }

#if ESP3D_TIMESTAMP_FEATURE
  // Time
  tmpstr = esp3dTimeService.getCurrentTime();
  tmpstr += "  (";
  tmpstr += esp3dTimeService.getTimeZone();
  tmpstr += ")";
  if (!dispatchIdValue(json, "time", tmpstr.c_str(), target, requestId)) {
    return;
  }
#endif  // ESP3D_TIMESTAMP_FEATURE

  // end of list
  if (json) {
    if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending answer to clients");
    }
  } else {
    {
      if (addPreTag) {
        tmpstr = "</pre>\n";
      } else {
        tmpstr = "ok\n";
      }
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
  }
}