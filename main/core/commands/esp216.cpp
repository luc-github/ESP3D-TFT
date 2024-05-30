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
#if LV_USE_SNAPSHOT && ESP3D_DISPLAY_FEATURE

#include <lvgl.h>

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_sd.h"

#define COMMAND_ID 216

// Do Snapshot of current screen
//[ESP216]<SNAP> json=<no> pwd=<user/admin password>
void ESP3DCommands::ESP216(int cmd_params_pos, ESP3DMessage* msg) {
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
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    hasError = true;
  } else {
    // TODO: do the snapshot
    lv_obj_t* currentt_scr = lv_scr_act();
    lv_img_dsc_t snapshot;
    uint32_t buffer_size =
        lv_snapshot_buf_size_needed(currentt_scr, LV_IMG_CF_TRUE_COLOR);
    esp3d_log("Snapshot buffer size needed: %ld", buffer_size);
    uint8_t* buffer_image = (uint8_t*)malloc(buffer_size);
    if (!buffer_image) {
      hasError = true;
      error_msg = "Not enough memory";
    } else {
      lv_snapshot_take_to_buf(currentt_scr, LV_IMG_CF_TRUE_COLOR, &snapshot,
                              buffer_image, buffer_size);
      if (sd.accessFS()) {
        std::string filename = "snapshot";
        filename += std::to_string(esp3d_hal::millis()) + ".raw";
        FILE* fd = sd.open(filename.c_str(), "w");
        if (fd) {
          if (fwrite(buffer_image, buffer_size, 1, fd) != 1) {
            hasError = true;
            error_msg = "write failed";
          }
          sd.close(fd);
        } else {
          hasError = true;
          error_msg = "creation failed";
        }
        sd.releaseFS();
      } else {
        hasError = true;
        error_msg = "access SD failed";
      }

      free(buffer_image);
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // LV_USE_SNAPSHOT && ESP3D_DISPLAY_FEATURE
