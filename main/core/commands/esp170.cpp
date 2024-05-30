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
#if ESP3D_CAMERA_FEATURE
#include "authentication/esp3d_authentication.h"
#include "camera/camera.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"

#define COMMAND_ID 170
// Set Camera command value / list all values in JSON/plain
//[ESP170]<json><label=value> pwd=<admin password>
// label can be:
// light/framesize/quality/contrast/brightness/saturation/gainceiling/colorbar
//              /awb/agc/aec/hmirror/vflip/awb_gain/agc_gain/aec_value/aec2/cw/bpc/wpc
//              /raw_gma/lenc/special_effect/wb_mode/ae_level
void ESP3DCommands::ESP170(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* camcmd[] = {
      "framesize", "quality",        "contrast", "brightness", "saturation",
      "sharpness", "special_effect", "wb_mode",  "awb",        "awb_gain",
      "aec",       "aec2",           "ae_level", "aec_value",  "agc",
      "agc_gain",  "gainceiling",    "bpc",      "wpc",        "raw_gma",
      "lenc",      "vflip",          "hmirror",  "dcw",        "colorbar",
      "light",
  };
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (!esp3d_camera.started()) {
    hasError = true;
    error_msg = "No camera initialized";
    esp3d_log_e("%s", error_msg.c_str());
  } else {
    tmpstr = get_clean_param(msg, cmd_params_pos);
    sensor_t* s = esp_camera_sensor_get();
    if (s == nullptr) {
      hasError = true;
      error_msg = "No camera settings available";
      esp3d_log_e("%s", error_msg.c_str());
    } else {
      if (tmpstr.length() == 0) {
        if (json) {
          tmpstr = "{\"cmd\":\"170\",\"status\":\"ok\",\"data\":[";

        } else {
          tmpstr = "Camera:\n";
        }
        msg->type = ESP3DMessageType::head;
        if (!dispatch(msg, tmpstr.c_str())) {
          esp3d_log_e("Error sending response to clients");
          return;
        }
        // now send all settings one by one
        //  framesize
        if (!dispatchIdValue(json, "framesize",
                             std::to_string(s->status.framesize).c_str(),
                             target, requestId, true)) {
          return;
        }

        // quality
        if (!dispatchIdValue(json, "quality",
                             std::to_string(s->status.quality).c_str(), target,
                             requestId)) {
          return;
        }

        // brightness
        if (!dispatchIdValue(json, "brightness",
                             std::to_string(s->status.brightness).c_str(),
                             target, requestId)) {
          return;
        }

        // contrast
        if (!dispatchIdValue(json, "contrast",
                             std::to_string(s->status.contrast).c_str(), target,
                             requestId)) {
          return;
        }

        // saturation
        if (!dispatchIdValue(json, "saturation",
                             std::to_string(s->status.saturation).c_str(),
                             target, requestId)) {
          return;

          // sharpness
          if (!dispatchIdValue(json, "sharpness",
                               std::to_string(s->status.sharpness).c_str(),
                               target, requestId)) {
            return;
          }

          // special_effect
          if (!dispatchIdValue(json, "special_effect",
                               std::to_string(s->status.special_effect).c_str(),
                               target, requestId)) {
            return;
          }

          // wb_mode
          if (!dispatchIdValue(json, "wb_mode",
                               std::to_string(s->status.wb_mode).c_str(),
                               target, requestId)) {
            return;
          }

          // awb
          if (!dispatchIdValue(json, "awb",
                               std::to_string(s->status.awb).c_str(), target,
                               requestId)) {
            return;
          }

          // awb_gain
          if (!dispatchIdValue(json, "awb_gain",
                               std::to_string(s->status.awb_gain).c_str(),
                               target, requestId)) {
            return;
          }

          // aec
          if (!dispatchIdValue(json, "aec",
                               std::to_string(s->status.aec).c_str(), target,
                               requestId)) {
            return;
          }
          // aec2
          if (!dispatchIdValue(json, "aec2",
                               std::to_string(s->status.aec2).c_str(), target,
                               requestId)) {
            return;
          }
          // ae_level
          if (!dispatchIdValue(json, "ae_level",
                               std::to_string(s->status.ae_level).c_str(),
                               target, requestId)) {
            return;
          }
          // aec_value
          if (!dispatchIdValue(json, "aec_value",
                               std::to_string(s->status.aec_value).c_str(),
                               target, requestId)) {
            return;
          }
          // agc
          if (!dispatchIdValue(json, "agc",
                               std::to_string(s->status.agc).c_str(), target,
                               requestId)) {
            return;
          }
          // agc_gain
          if (!dispatchIdValue(json, "agc_gain",
                               std::to_string(s->status.agc_gain).c_str(),
                               target, requestId)) {
            return;
          }
          // gainceiling
          if (!dispatchIdValue(json, "gainceiling",
                               std::to_string(s->status.gainceiling).c_str(),
                               target, requestId)) {
            return;
          }
          // bpc
          if (!dispatchIdValue(json, "bpc",
                               std::to_string(s->status.bpc).c_str(), target,
                               requestId)) {
            return;
          }
          // wpc
          if (!dispatchIdValue(json, "wpc",
                               std::to_string(s->status.wpc).c_str(), target,
                               requestId)) {
            return;
          }
          // raw_gma
          if (!dispatchIdValue(json, "raw_gma",
                               std::to_string(s->status.raw_gma).c_str(),
                               target, requestId)) {
            return;
          }
          // lenc
          if (!dispatchIdValue(json, "lenc",
                               std::to_string(s->status.lenc).c_str(), target,
                               requestId)) {
            return;
          }
          // vflip
          if (!dispatchIdValue(json, "vflip",
                               std::to_string(s->status.vflip).c_str(), target,
                               requestId)) {
            return;
          }
          // hmirror
          if (!dispatchIdValue(json, "hmirror",
                               std::to_string(s->status.hmirror).c_str(),
                               target, requestId)) {
            return;
          }
          // dcw
          if (!dispatchIdValue(json, "dcw",
                               std::to_string(s->status.dcw).c_str(), target,
                               requestId)) {
            return;
          }
          // colorbar
          if (!dispatchIdValue(json, "colorbar",
                               std::to_string(s->status.colorbar).c_str(),
                               target, requestId)) {
            return;
          }
        }
        if (json) {
          if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
            esp3d_log_e("Error sending answer to clients");
          }
        } else {
          {
            tmpstr = "ok\n";
            if (!dispatch(tmpstr.c_str(), target, requestId,
                          ESP3DMessageType::tail)) {
              esp3d_log_e("Error sending answer to clients");
            }
          }
        }
        return;
      } else {
        size_t s = sizeof(camcmd) / sizeof(const char*);
        for (size_t i = 0; i < s; i++) {
          std::string label = camcmd[i];
          label += "=";
          tmpstr = get_param(msg, cmd_params_pos, label.c_str());
          if (tmpstr.length() > 0) {
            int r = esp3d_camera.command(camcmd[i], tmpstr.c_str());
            if (r == -1) {
              hasError = true;
              error_msg = "Unknow command";
              esp3d_log_e("%s", error_msg.c_str());
            } else if (r == 1) {
              hasError = true;
              error_msg = "Invalid value";
              esp3d_log_e("%s", error_msg.c_str());
            } else {
              ok_msg = "ok";
            }
          }
        }
      }
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_CAMERA_FEATURE