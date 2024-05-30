/*
  esp3d_commands

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

#pragma once

#include <stdio.h>

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_settings.h"
#if ESP3D_SD_CARD_FEATURE
#include "sd_def.h"
#endif  // ESP3D_SD_CARD_FEATURE

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DCommands {
 public:
  ESP3DCommands();
  ~ESP3DCommands();
  bool is_esp_command(uint8_t* sbuf, size_t len);
  void process(ESP3DMessage* msg);
  void execute_internal_command(int cmd, int cmd_params_pos, ESP3DMessage* msg);
  bool dispatchAnswer(ESP3DMessage* msg, uint cmdid, bool json, bool hasError,
                      const char* answerMsg);
  bool dispatchIdValue(bool json, const char* Id, const char* value,
                       ESP3DClientType target, ESP3DRequest requestId,
                       bool isFirst = false);
  bool dispatchKeyValue(bool json, const char* key, const char* value,
                        ESP3DClientType target, ESP3DRequest requestId,
                        bool nested = false, bool isFirst = false);
  bool dispatchSetting(bool json, const char* filter, ESP3DSettingIndex index,
                       const char* help, const char** optionValues,
                       const char** optionLabels, uint32_t maxsize,
                       uint32_t minsize, uint32_t minsize2, uint8_t precision,
                       const char* unit, bool needRestart,
                       ESP3DClientType target, ESP3DRequest requestId,
                       bool isFirst = false);
  bool dispatch(ESP3DMessage* msg);
  bool dispatch(ESP3DMessage* msg, uint8_t* sbuf, size_t len);
  bool dispatch(ESP3DMessage* msg, const char* sbuf);
  bool dispatch(const char* sbuf, ESP3DClientType target,
                ESP3DRequest requestId,
                ESP3DMessageType type = ESP3DMessageType::head,
                ESP3DClientType origin = ESP3DClientType::command,
                ESP3DAuthenticationLevel authentication_level =
                    ESP3DAuthenticationLevel::guest);
  bool dispatchAuthenticationError(ESP3DMessage* msg, uint cmdid, bool json);
  bool formatCommand(char* cmd, size_t len);
  void ESP0(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_WIFI_FEATURE
  void ESP100(int cmd_params_pos, ESP3DMessage* msg);
  void ESP101(int cmd_params_pos, ESP3DMessage* msg);
  void ESP102(int cmd_params_pos, ESP3DMessage* msg);
  void ESP103(int cmd_params_pos, ESP3DMessage* msg);
  void ESP104(int cmd_params_pos, ESP3DMessage* msg);
  void ESP105(int cmd_params_pos, ESP3DMessage* msg);
  void ESP106(int cmd_params_pos, ESP3DMessage* msg);
  void ESP107(int cmd_params_pos, ESP3DMessage* msg);
  void ESP108(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_WIFI_FEATURE
  void ESP110(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_WIFI_FEATURE
  void ESP111(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_WIFI_FEATURE
  void ESP112(int cmd_params_pos, ESP3DMessage* msg);
  void ESP114(int cmd_params_pos, ESP3DMessage* msg);
  void ESP115(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_HTTP_FEATURE
  void ESP120(int cmd_params_pos, ESP3DMessage* msg);
  void ESP121(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
  void ESP130(int cmd_params_pos, ESP3DMessage* msg);
  void ESP131(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
  void ESP140(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_HTTP_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
  void ESP160(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_CAMERA_FEATURE
  void ESP170(int cmd_params_pos, ESP3DMessage* msg);
  void ESP171(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_CAMERA_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
  void ESP190(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_SD_CARD_FEATURE
  void ESP200(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_SD_IS_SPI
  void ESP202(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_SD_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_DISPLAY_FEATURE
  void ESP214(int cmd_params_pos, ESP3DMessage* msg);
#if LV_USE_SNAPSHOT
  void ESP216(int cmd_params_pos, ESP3DMessage* msg);
#endif  // LV_USE_SNAPSHOT
#endif  // ESP3D_DISPLAY_FEATURE
  void ESP400(int cmd_params_pos, ESP3DMessage* msg);
  void ESP401(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
  void ESP402(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_WIFI_FEATURE
  void ESP410(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_WIFI_FEATURE
  void ESP420(int cmd_params_pos, ESP3DMessage* msg);
  void ESP444(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_MDNS_FEATURE
  void ESP450(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_MDNS_FEATURE
  void ESP460(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_AUTHENTICATION_FEATURE
  void ESP500(int cmd_params_pos, ESP3DMessage* msg);
  void ESP510(int cmd_params_pos, ESP3DMessage* msg);
  void ESP550(int cmd_params_pos, ESP3DMessage* msg);
  void ESP555(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
  void ESP600(int cmd_params_pos, ESP3DMessage* msg);
  void ESP610(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_NOTIFICATIONS_FEATURE
  void ESP700(int cmd_params_pos, ESP3DMessage* msg);
  void ESP701(int cmd_params_pos, ESP3DMessage* msg);
  void ESP702(int cmd_params_pos, ESP3DMessage* msg);
  void ESP710(int cmd_params_pos, ESP3DMessage* msg);
  void ESP720(int cmd_params_pos, ESP3DMessage* msg);
  void ESP730(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_SD_CARD_FEATURE
  void ESP740(int cmd_params_pos, ESP3DMessage* msg);
  void ESP750(int cmd_params_pos, ESP3DMessage* msg);
#endif  // ESP3D_SD_CARD_FEATURE
  void ESP780(int cmd_params_pos, ESP3DMessage* msg);
  void ESP790(int cmd_params_pos, ESP3DMessage* msg);
  void ESP800(int cmd_params_pos, ESP3DMessage* msg);
  void ESP900(int cmd_params_pos, ESP3DMessage* msg);
  void ESP901(int cmd_params_pos, ESP3DMessage* msg);
#if ESP3D_USB_SERIAL_FEATURE
  void ESP902(int cmd_params_pos, ESP3DMessage* msg);
  void ESP950(int cmd_params_pos, ESP3DMessage* msg);
#endif  // #if ESP3D_USB_SERIAL_FEATURE
  const char* get_param(ESP3DMessage* msg, uint start, const char* label,
                        bool* found = nullptr);
  const char* get_param(const char* data, uint size, uint start,
                        const char* label, bool* found = nullptr);
  const char* get_clean_param(ESP3DMessage* msg, uint start);
  bool has_param(ESP3DMessage* msg, uint start);
  bool hasTag(ESP3DMessage* msg, uint start, const char* label);
  void flush();
  ESP3DClientType getOutputClient(bool fromSettings = false);
  void setOutputClient(ESP3DClientType output_client) {
    _output_client = output_client;
  }

 private:
  ESP3DClientType _output_client;
};

extern ESP3DCommands esp3dCommands;

#ifdef __cplusplus
}  // extern "C"
#endif
