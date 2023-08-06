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
#if ESP3D_MDNS_FEATURE
#include <stdio.h>

#include <cstring>
#include <string>

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "esp_netif_ip_addr.h"
#include "mdns/esp3d_mdns.h"
#include "network/esp3d_network.h"

#define COMMAND_ID 450
// Get available ESP3D list
// output is JSON or plain text according parameter
//[ESP450]json=<no> <pwd=admin/user>
void ESP3DCommands::ESP450(int cmd_params_pos, ESP3DMessage* msg) {
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

  if (esp3dNetwork.getMode() == ESP3DRadioMode::off ||
      esp3dNetwork.getMode() == ESP3DRadioMode::bluetooth_serial) {
    tmpstr = "Network not enabled";
    dispatchAnswer(msg, COMMAND_ID, json, true, tmpstr.c_str());
    return;
  }

  if (json) {
    tmpstr = "{\"cmd\":\"450\",\"status\":\"ok\",\"data\":[";

  } else {
    tmpstr = "Start Scan\n";
  }
  msg->type = ESP3DMessageType::head;
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }

  uint16_t count = esp3dmDNS.servicesScan();
  esp3d_log("Total : %d", count);
  for (uint16_t i = 0; i < count; i++) {
    mdns_result_t* result = esp3dmDNS.getRecord();
    if (result) {
      // Currently esp3d support only IPV4 and only one address per device
      std::string ip =
          ip4addr_ntoa((const ip4_addr_t*)&result->addr->addr.u_addr.ip4);
      esp3d_log("hostname: %s IP:%s port:%d ", result->hostname, ip.c_str(),
                result->port);

      tmpstr = "";

      if (json) {
        if (i > 0) {
          tmpstr += ",";
        }

        tmpstr += "{\"Hostname\":\"";
      }
      tmpstr += result->hostname;
      if (json) {
        tmpstr += "\",\"IP\":\"";
      } else {
        tmpstr += " (";
      }
      tmpstr += ip;
      if (json) {
        tmpstr += "\",\"port\":\"";
        tmpstr += std::to_string(result->port);
        tmpstr += "\",\"TxT\":[";

      } else {
        tmpstr += ":";
        tmpstr += std::to_string(result->port);
        tmpstr += ") ";
      }
      for (uint8_t t = 0; t < result->txt_count; t++) {
        if (t != 0) {
          tmpstr += ",";
        }
        if (json) {
          tmpstr += "{\"key\":\"";
        }
        tmpstr += result->txt[t].key;
        if (json) {
          tmpstr += "\",\"value\":\"";
        } else {
          tmpstr += "=";
        }
        tmpstr += result->txt[t].value;
        if (json) {
          tmpstr += "\"}";
        }
      }

      if (json) {
        tmpstr += "]}";
      } else {
        tmpstr += "\n";
      }
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::core)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
  }

  esp3dmDNS.freeServiceScan();

  // end of list
  if (json) {
    tmpstr = "]}";
  } else {
    tmpstr = "End Scan\n";
  }
  if (!dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::tail)) {
    esp3d_log_e("Error sending answer to clients");
  }
}
#endif  // ESP3D_MDNS_ENABLED
