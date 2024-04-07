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
#include "esp3d_hal.h"
#include "esp3d_string.h"
#include "gcode_host/esp3d_gcode_host_service.h"

#define COMMAND_ID 701

// Query and Control ESP700 stream
//[ESP701]action=<PAUSE/RESUME/ABORT> json=<no> pwd=<admin/user password>`
void ESP3DCommands::ESP701(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_param(msg, cmd_params_pos, "action=");
  ESP3DGcodeHostState status = gcodeHostService.getState();
  ESP3DGcodeHostError errorNum = gcodeHostService.getErrorNum();
  if (tmpstr.length() == 0) {
    if (status == ESP3DGcodeHostState::idle) {
      if (json) {
        ok_msg = "{\"status\":\"no stream\"";
        if (errorNum != ESP3DGcodeHostError::no_error) {
          ok_msg += ",\"code\":\"";
          ok_msg += std::to_string(static_cast<uint8_t>(errorNum));
          ok_msg += "\"";
        }
        ok_msg += "}";
      } else {
        ok_msg = "no stream";
        if (errorNum != ESP3DGcodeHostError::no_error) {
          ok_msg += ", last error ";
          ok_msg += std::to_string(static_cast<uint8_t>(errorNum));
        }
      }
    } else {
      if (status != ESP3DGcodeHostState::idle) {
        ESP3DGcodeStream* script = gcodeHostService.getCurrentMainStream();
        if (script == NULL) {
          hasError = true;
          error_msg = "Failed to get script";
        } else {
          if (status == ESP3DGcodeHostState::paused) {
            if (json) {
              ok_msg = "{\"status\":\"pause\"";
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
            ok_msg += std::to_string(script->totalSize);
            ok_msg += "\",\"processed\":\"";
            ok_msg += std::to_string(script->processedSize);
            ok_msg += "\",\"elapsed\":\"";
            ok_msg += std::to_string(esp3d_hal::millis() - script->id);
            ok_msg += "\",\"type\":\"";
            ok_msg += std::to_string(static_cast<uint8_t>(script->type));
            if (script->type == ESP3DGcodeHostStreamType::sd_stream ||
                script->type == ESP3DGcodeHostStreamType::fs_stream) {
              ok_msg += "\",\"name\":\"";
              ok_msg += ((ESP3DGcodeStream*)script)->dataStream;
            }
            ok_msg += "\"}";
          } else {
            // TODO: add more info ?
          }
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
      }
    } else {
      hasError = true;
      error_msg = "Invalid parameters";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
