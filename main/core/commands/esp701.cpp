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
#if ESP3D_GCODE_HOST_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "gcode_host/esp3d_gcode_host_service.h"

#define COMMAND_ID 701

// Query and Control ESP700 stream
//[ESP701]action=<PAUSE/RESUME/ABORT> json=<no> pwd=<admin/user password>`
void Esp3DCommands::ESP701(int cmd_params_pos, esp3d_msg_t* msg) {
  Esp3dClient target = msg->origin;
  esp3d_request_t requestId = msg->requestId;
  (void)requestId;
  msg->target = target;
  msg->origin = Esp3dClient::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == Esp3dAuthenticationLevel::guest) {
    msg->authentication_level = Esp3dAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_param(msg, cmd_params_pos, "action=");
  Esp3dGcodeHostState status = gcodeHostService.getState();
  Esp3dGcodeHostError errorNum = gcodeHostService.getErrorNum();
  if (tmpstr.length() == 0) {
    if (status == Esp3dGcodeHostState::no_stream) {
      if (json) {
        ok_msg = "{\"status\":\"no stream\"";
        if (errorNum != Esp3dGcodeHostError::no_error) {
          ok_msg += "\",\"code\":\"";
          ok_msg += std::to_string(static_cast<uint8_t>(errorNum));
          ok_msg += "\"";
        }
        ok_msg += "}";
      } else {
        ok_msg = "no stream";
        if (errorNum != Esp3dGcodeHostError::no_error) {
          ok_msg += ", last error ";
          ok_msg += std::to_string(static_cast<uint8_t>(errorNum));
        }
      }
    } else {
      Esp3dScript* script = gcodeHostService.getCurrentScript();
      if (script) {
        if (status == Esp3dGcodeHostState::paused) {
          if (json) {
            ok_msg = "{\"status\":\"paused\"";
          } else {
            ok_msg = "pause";
          }
        } else {
          if (json) {
            ok_msg = "{\"status\":\"processing\"";
          } else {
            ok_msg = "processing";
          }
        }
        if (json) {
          ok_msg += ",\"total\":\"";
          ok_msg += std::to_string(script->total);
          ok_msg += "\",\"processed\":\"";
          ok_msg += std::to_string(script->progress);
          ok_msg += "\",\"type\":\"";
          ok_msg += std::to_string(static_cast<uint8_t>(script->type));
          if (script->type == Esp3dGcodeHostScriptType::sd_card ||
              script->type == Esp3dGcodeHostScriptType::filesystem) {
            ok_msg += "\",\"name\":\"";
            ok_msg += script->script;
            ok_msg += "\"";
          }
          ok_msg += "\"}";
        } else {
          // Nothing to say ?
        }
      } else {
        hasError = true;
        error_msg = "Failed to get script";
      }
    }
  } else {
    // send command PAUSE/RESUME/ABORT
    if (tmpstr == "PAUSE") {
      if (!gcodeHostService.pause()) {
        hasError = true;
        error_msg = "Failed to pause";
      }
    } else if (tmpstr == "RESUME") {
      if (!gcodeHostService.resume()) {
        hasError = true;
        error_msg = "Failed to resume";
      }
    } else if (tmpstr == "ABORT") {
      if (!gcodeHostService.abort()) {
        hasError = true;
        error_msg = "Failed to abort";
      } else {
        hasError = true;
        error_msg = "Invalid parameters";
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_GCODE_HOST_FEATURE