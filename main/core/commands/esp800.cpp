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
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "filesystem/esp3d_flash.h"
#include "network/esp3d_network.h"
#include "sdkconfig.h"
#include "spi_flash_mmap.h"
#if ESP3D_UPDATE_FEATURE
#include "update/esp3d_update_service.h"
#endif  // ESP3D_UPDATE_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
#include "time/esp3d_time_service.h"
#endif  // ESP3D_TIMESTAMP_FEATURE
#ifdef ESP3D_CAMERA_FEATURE
#include "camera/camera.h"
#endif  // ESP3D_CAMERA_FEATURE

#define COMMAND_ID 800

// get fw version firmare target and fw version
// eventually set time with pc time
// output is JSON or plain text according parameter
//[ESP800]json=<no> <time=YYYY-MM-DDTHH:mm:ss> <version=3.0.0-a11> <setup=0/1>
void ESP3DCommands::ESP800(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  std::string timestr = "none";
#if ESP3D_TIMESTAMP_FEATURE
  std::string timeparam = get_param(msg, cmd_params_pos, "time=");
  std::string tzparam = get_param(msg, cmd_params_pos, "tz=");
#endif  // ESP3D_TIMESTAMP_FEATURE
  std::string setupparam = get_param(msg, cmd_params_pos, "setup=");
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
  // set time if internet time is not enabled
  if (!esp3dTimeService.isInternetTime()) {
    if (tzparam.length() > 0) {
      if (!esp3dTimeService.setTimeZone(tzparam.c_str())) {
        // not blocking error
        esp3d_log_e("Error setting timezone");
        timestr = "Failed to set timezone";
      } else {
        timestr = "Manual";
      }
    } else {
      timestr = "Not set";
    }
    if (timeparam.length() > 0) {
      if (!esp3dTimeService.setTime(timeparam.c_str())) {
        // not blocking error
        esp3d_log_e("Error setting time");
        timestr = "Failed to set time";
      }
    }
  } else {
    if (esp3dTimeService.started()) {
      timestr = "Auto";
    } else {
      timestr = "Failed to set";
    }
  }
#else
  timestr = "none";
#endif  // ESP3D_TIMESTAMP_FEATURE

  if (setupparam.length() > 0) {
    if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_setup,
                                    setupparam == "1" ? 1 : 0)) {
      // not blocking error
      esp3d_log_e("Error writing setup state");
    }
  }
  if (json) {
    tmpstr = "{\"cmd\":\"800\",\"status\":\"ok\",\"data\":{";

  } else {
    tmpstr = "Capabilities:\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  // ESP3D-TFT-VERSION
  if (!dispatchKeyValue(json, "FWVersion", ESP3D_TFT_VERSION, target, requestId,
                        false, true)) {
    return;
  }
  uint8_t b =
      esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_target_firmware);
  // FWTarget
  tmpstr = esp3dTftsettings.GetFirmwareTargetShortName((ESP3DTargetFirmware)b);
  if (!dispatchKeyValue(json, "FWTarget", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // FWTargetID
  tmpstr = std::to_string(b);
  if (!dispatchKeyValue(json, "FWTargetID", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }

  // Setup
  tmpstr = esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_setup) == 1
               ? "Enabled"
               : "Disabled";
  if (!dispatchKeyValue(json, "Setup", tmpstr.c_str(), target, requestId)) {
    return;
  }
  std::string sdconnection = "none";
#if ESP3D_SD_CARD_FEATURE
  sdconnection = "direct";
#endif  // ESP3D_SD_CARD_FEATURE

  // SD connection
  if (!dispatchKeyValue(json, "SDConnection", sdconnection.c_str(), target,
                        requestId)) {
    return;
  }

  // Serial Protocol
  if (!dispatchKeyValue(json, "SerialProtocol", "Raw", target, requestId)) {
    return;
  }

  // Authentication
#if ESP3D_AUTHENTICATION_FEATURE
  tmpstr = "Enabled";
#else
  tmpstr = "Disabled";
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (!dispatchKeyValue(json, "Authentication", tmpstr.c_str(), target,
                        requestId)) {
    return;
  }
#if ESP3D_HTTP_FEATURE
  // WebCommunication
  if (!dispatchKeyValue(json, "WebCommunication", "Asynchronous", target,
                        requestId)) {
    return;
  }

  // WebSocketIP
  ESP3DIpInfos ipInfo;
  if (esp3dNetwork.getLocalIp(&ipInfo)) {
    tmpstr = ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
    if (!dispatchKeyValue(json, "WebSocketIP", tmpstr.c_str(), target,
                          requestId)) {
      return;
    }
    uint32_t intValue =
        esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_http_port);
    tmpstr = std::to_string(intValue);
    if (!dispatchKeyValue(json, "WebSocketPort", tmpstr.c_str(), target,
                          requestId)) {
      return;
    }
  }
#endif  // ESP3D_HTTP_FEATURE
  // Hostname
  const ESP3DSettingDescription* settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_hostname);
  if (settingPtr) {
    char out_str[(settingPtr->size) + 1] = {0};
    tmpstr = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_hostname,
                                         out_str, settingPtr->size);
  } else {
    tmpstr = "Error!!";
  }
  if (!dispatchKeyValue(json, "Hostname", tmpstr.c_str(), target, requestId)) {
    return;
  }

  // WiFiMode
  switch (esp3dNetwork.getMode()) {
#if ESP3D_WIFI_FEATURE
    case ESP3DRadioMode::wifi_sta:
      tmpstr = "STA";
      break;
    case ESP3DRadioMode::wifi_ap:
      tmpstr = "AP";
      break;
    case ESP3DRadioMode::wifi_ap_config:
      tmpstr = "CONFIG";
      break;
#endif  // ESP3D_WIFI_FEATURE
    case ESP3DRadioMode::off:
      tmpstr = "RADIO OFF";
      break;
    case ESP3DRadioMode::bluetooth_serial:
      tmpstr = "BT";
      break;
    default:
      tmpstr = "Unknown";
  }
  if (!dispatchKeyValue(json, "RadioMode", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // WebUpdate
  std::string canupdate = "Disabled";
#if ESP3D_UPDATE_FEATURE
  canupdate = esp3dUpdateService.canUpdate() ? "Enabled" : "Disabled";
#endif  // ESP3D_UPDATE_FEATURE

  if (!dispatchKeyValue(json, "WebUpdate", canupdate.c_str(), target,
                        requestId)) {
    return;
  }
  // FlashFileSystem
  if (!dispatchKeyValue(json, "FlashFileSystem", "LittleFS", target,
                        requestId)) {
    return;
  }
  // HostPath
  if (!dispatchKeyValue(json, "HostPath", "/", target, requestId)) {
    return;
  }

  // Screen
  if (!dispatchKeyValue(json, "Screen", TFT_TARGET, target, requestId)) {
    return;
  }

  // Streaming
  if (!dispatchKeyValue(json, "Streaming", "Enabled", target, requestId)) {
    return;
  }

#if ESP3D_TIMESTAMP_FEATURE

#endif  // ESP3D_TIMESTAMP_FEATURE
  // Time
  if (!dispatchKeyValue(json, "Time", timestr.c_str(), target, requestId)) {
    return;
  }

#ifdef ESP3D_CAMERA_FEATURE
  // camera ID

  tmpstr = esp3d_camera.GetModel();
  if (!dispatchKeyValue(json, "CameraID", tmpstr.c_str(), target, requestId)) {
    return;
  }
  // camera Name
  if (!dispatchKeyValue(json, "CameraName", esp3d_camera.GetModelString(),
                        target, requestId)) {
    return;
  }
#endif  // ESP3D_CAMERA_FEATURE

  // end of list
  if (json) {
    if (!dispatch("}}", target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending answer to clients");
    }
  } else {
    {
      if (!dispatch("ok\n", target, requestId, ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
  }
}
