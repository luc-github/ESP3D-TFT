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
#if ESP3D_WIFI_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "network/esp3d_network.h"

#define COMMAND_ID 111
// Get current IP
//[ESP111]<ALL> [json=no]
void ESP3DCommands::ESP111(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool showAll = hasTag(msg, cmd_params_pos, "ALL");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() != 0 && !showAll) {
    hasError = true;
  } else {
    ESP3DIpInfos ipInfo;
    if (esp3dNetwork.getLocalIp(&ipInfo)) {
      ok_msg = ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
    } else {
      hasError = true;
      error_msg = "Cannot get Ip";
    }
    if (showAll && !hasError) {
      if (json) {
        ok_msg = "{\"ip\":\"";
      } else {
        ok_msg = "IP: ";
      }
      ok_msg += ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
      if (json) {
        ok_msg += "\",\"gw\":\"";
      } else {
        ok_msg += "\nGW: ";
      }
      ok_msg += ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.gw));
      if (json) {
        ok_msg += "\",\"msk\":\"";
      } else {
        ok_msg += "\nMSK: ";
      }
      ok_msg += ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.netmask));
      if (json) {
        ok_msg += "\",\"dns\":\"";
      } else {
        ok_msg += "\nDNS: ";
      }
      ok_msg +=
          ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.dns_info.ip.u_addr.ip4));
      if (json) {
        ok_msg += "\"}";
      } else {
        ok_msg += "\n";
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_WIFI_FEATURE