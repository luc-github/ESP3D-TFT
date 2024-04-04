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

#include "esp3d_client.h"
#include "esp3d_commands.h"
#if ESP3D_SD_CARD_FEATURE
#include "sd_def.h"
#endif  // ESP3D_SD_CARD_FEATURE

const char* help[] = {
    "[ESP](id) - display this help",
#if ESP3D_WIFI_FEATURE
    "[ESP100](SSID) - display/set STA SSID",
    "[ESP101](Password)/(NOPASSWORD)- set STA password",
    "[ESP102](Mode) - display/set STA IP mode (DHCP/STATIC)",
    "[ESP103](IP=xxxx MSK=xxxx GW=xxxx DNS=XXXXX) - display/set STA "
    "IP/Mask/GW/DNS",
    "[ESP104](State) - display/set sta fallback mode which can be CONFIG, BT, "
    "OFF",
    "[ESP105](SSID) - display/set AP SSID",
    "[ESP106](Password)/(NOPASSWORD) - set AP password",
    "[ESP107](IP) - display/set AP IP",
    "[ESP108](Chanel) - display/set AP chanel",
#endif  // ESP3D_WIFI_FEATURE
    "[ESP110](State) - display/set radio state which can be STA,AP, CONFIG, BT "
    "OFF",
#if ESP3D_WIFI_FEATURE
    "[ESP111] - display current IP",
#endif  // ESP3D_WIFI_FEATURE
    "[ESP112](Hostname) - display/set Hostname",
    "[ESP114](State) - display/set boot Radio state which can be ON, OFF",
    "[ESP115](State) - display/set immediate Radio state which can be ON, OFF",
#if ESP3D_HTTP_FEATURE
    "[ESP120](State) - display/set HTTP state which can be ON, OFF",
    "[ESP121](Port) - display/set HTTP port",
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
    "[ESP130](State) - display/set Telnet state which can be ON, OFF, CLOSE",
    "[ESP131](Port) - display/set Telnet port ",
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
    "[ESP140](sync) (srv(x)=XXXXX) (tzone=+/-HH:mm)(time=YYYY-MM-DDTHH:mm:ss) "
    "(ntp=yes/no) "
    "(now) - display/set current time",
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_HTTP_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    "[ESP160](State) - display/set WebSocket state which can be ON, OFF, CLOSE",
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
    "[ESP190](State) - display/set WebDav state which can be ON, OFF",
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#if ESP3D_CAMERA_FEATURE
    "[ESP170](json) (label=value) - display/set Camera commands",
    "[ESP171] (path=<target path>) (filename=<target filename>) Save frame to "
    "target path and filename",
#endif  // ESP3D_CAMERA_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_SD_CARD_FEATURE
    "[ESP200](RELEASE) (REFRESH)- display/set SD Card Status",
#if ESP3D_SD_IS_SPI
    "[ESP202](factor) - display / set  SD Card  SD card Speed divider factor "
    "(1 2 4 6 8 16 32)",
#endif  // ESP3D_SD_IS_SPI
#if ESP3D_DISPLAY_FEATURE
    "[ESP214](Text) - Output to esp screen status",
#if LV_USE_SNAPSHOT
    "[ESP216](SNAP) - Do Snapshot of current screen",
#endif  // LV_USE_SNAPSHOT
#endif  // ESP3D_DISPLAY_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
    "[ESP400] - display ESP3D settings",
    "[ESP401]P=(position) T=(type) V=(value) - Set specific setting",
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
    "[ESP402](State) - display/set Check update at boot state which can be ON, "
    "OFF",
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_WIFI_FEATURE
    "[ESP410] - display available AP list",
#endif  // ESP3D_WIFI_FEATURE
    "[ESP420] - display ESP3D current status",
    "[ESP444](state) - set ESP3D state (RESET/RESTART)",
#if ESP3D_MDNS_FEATURE
    "[ESP450]display ESP3D list on network",
#endif  // #if ESP3D_MDNS_FEATURE
    "[ESP460](language code) - display/set UI language code",
#if ESP3D_AUTHENTICATION_FEATURE
    "[ESP500] - set get connection status",
    "[ESP510](timeout) - display/set session timeout",
    "[ESP550](password) - set admin password",
    "[ESP555](password) - set user password",
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    "[ESP600](message) - send notification",
    "[ESP610](type=NONE/PUSHOVER/EMAIL/LINE/IFTTT) (AUTO=YES/NO) (T1=token1) "
    "(T2=token2) (TS=Settings)",
#endif  // ESP3D_NOTIFICATIONS_FEATURE
    "[ESP700](stream=file name) or (macro name) - read and process/stream "
    "file/macro",
    "[ESP701]action=(PAUSE/RESUME/ABORT) - query and control ESP700 stream",
    "[ESP702](pause/stop/resume)=(script) - display/set ESP700 stream scripts",
    "[ESP710]FORMATFS - Format ESP3D Filesystem",
    "[ESP720](path) - List ESP3D Filesystem",
    "[ESP730](Action)=(path) - rmdir / remove / mkdir / exists / create on "
    "ESP3D FileSystem (path)",
#if ESP3D_SD_CARD_FEATURE
    "[ESP740](path) - List SD Filesystem",
    "[ESP750](Action)=(path) - rmdir / remove / mkdir / exists / create on SD "
    "(path)",
#endif  // ESP3D_SD_CARD_FEATURE
    "[ESP780](path) - List Global Filesystem",
    "[ESP790](Action)=(path) - rmdir / remove / mkdir / exists / create on "
    "Global FileSystem (path)",
    "[ESP800](time=YYYY-MM-DDTHH:mm:ss) (tz=+HH:SS) (version= webui "
    "version)(setup=0/1) - "
    "display FW Informations /set time",
    "[ESP900](state) - display/set serial state(ENABLE/DISABLE)",
    "[ESP901](baud rate) - display/set serial baud rate",
#if ESP3D_USB_SERIAL_FEATURE
    "[ESP902](baud rate) - display/set usb-serial baud rate",
    "[ESP950]<SERIAL/USB>  - display/set usb-serial client output",
#endif  // #if ESP3D_USB_SERIAL_FEATURE
};

const uint cmdlist[] = {
    0,
#if ESP3D_WIFI_FEATURE
    100, 101, 102, 103, 104, 105, 106, 107, 108,
#endif  // ESP3D_WIFI_FEATURE
    110,
#if ESP3D_WIFI_FEATURE
    111,
#endif  // ESP3D_WIFI_FEATURE
    112, 114, 115,
#if ESP3D_HTTP_FEATURE
    120, 121,
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
    130, 131,
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
    140,
#endif  // ESP3D_TIMESTAMP_FEATURE
#if ESP3D_HTTP_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    160,
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_CAMERA_FEATURE
    170, 171,
#endif  // ESP3D_CAMERA_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
    190,
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_SD_CARD_FEATURE
    200,
#if ESP3D_SD_IS_SPI
    202,
#endif  // ESP3D_SD_IS_SPI
#if ESP3D_DISPLAY_FEATURE
    214,
#if LV_USE_SNAPSHOT
    216,
#endif  // LV_USE_SNAPSHOT
#endif  // ESP3D_DISPLAY_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE

    400, 401,
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
    402,
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_WIFI_FEATURE
    410,
#endif  // ESP3D_WIFI_FEATURE
    420, 444,
#if ESP3D_MDNS_FEATURE
    450,
#endif  // ESP3D_MDNS_FEATURE
    460,
#if ESP3D_AUTHENTICATION_FEATURE
    500, 510, 550, 555,
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    600, 610,
#endif  // ESP3D_NOTIFICATIONS_FEATURE
    700, 701, 702, 710, 720, 730,
#if ESP3D_SD_CARD_FEATURE
    740, 750,
#endif  // ESP3D_SD_CARD_FEATURE

    780, 790, 800, 900, 901,
#if ESP3D_USB_SERIAL_FEATURE
    902, 950,
#endif  // #if ESP3D_USB_SERIAL_FEATURE
};
// ESP3D Help
//[ESP0] or [ESP]<command>
void ESP3DCommands::ESP0(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  std::string tmpstr;
  const uint cmdNb = sizeof(help) / sizeof(char*);
  const uint cmdlistNb = sizeof(cmdlist) / sizeof(uint);
  bool json = hasTag(msg, cmd_params_pos, "json");
  if (cmdNb != cmdlistNb) {
    esp3d_log("Help corrupted: %d vs %d", cmdNb, cmdlistNb);
    msg->type = ESP3DMessageType::unique;
    if (!dispatch(msg, "Help corrupted")) {
      esp3d_log_e("Error sending command to clients");
    }
    return;
  }
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    // use msg for first answer
    if (json) {
      tmpstr = "{\"cmd\":\"0\",\"status\":\"ok\",\"data\":[";
    } else {
      tmpstr = "[List of ESP3D commands]\n";
    }
    msg->type = ESP3DMessageType::head;
    if (!dispatch(msg, tmpstr.c_str())) {
      esp3d_log_e("Error sending command to clients");
      return;
    }
    for (uint i = 0; i < cmdNb; i++) {
      if (json) {
        tmpstr = "{\"id\":\"";
        tmpstr += std::to_string(cmdlist[i]);
        tmpstr += "\",\"help\":\"";
        tmpstr += help[i];
        tmpstr += "\"}";
        if (i < cmdNb - 1 && (cmdNb - 1) > 0) {
          tmpstr += ",";
        }
      } else {
        tmpstr = help[i];
        tmpstr += "\n";
      }
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::core)) {
        esp3d_log_e("Error sending answer to clients");
        return;
      }
    }

    if (json) {
      if (!dispatch("]}", target, requestId, ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
        return;
      }
    } else {
      if (!dispatch("ok\n", target, requestId, ESP3DMessageType::tail)) {
        esp3d_log_e("Error sending answer to clients");
        return;
      }
    }
  } else {
    uint cmdval = atoi(tmpstr.c_str());
    for (uint i = 0; i < cmdNb; i++) {
      if (cmdlist[i] == cmdval) {
        if (json) {
          tmpstr = "{\"cmd\":\"0\",\"status\":\"ok\",\"data\":{\"id\":\"";
          tmpstr += std::to_string(cmdval);
          tmpstr += "\",\"help\":\"";
          tmpstr += help[i];
          tmpstr += "\"}}";
        } else {
          tmpstr = help[i];
          tmpstr += "\n";
        }
        msg->type = ESP3DMessageType::unique;
        if (!dispatch(msg, tmpstr.c_str())) {
          return;
        }
        return;
      }
    }
    if (json) {
      tmpstr =
          "{\"cmd\":\"0\",\"status\":\"error\",\"data\":\"This command is not "
          "supported: ";
      tmpstr += std::to_string(cmdval);
      tmpstr += "\"}";
    } else {
      tmpstr = "This command is not supported: ";
      tmpstr += std::to_string(cmdval);
      tmpstr += "\n";
    }
    msg->type = ESP3DMessageType::unique;
    if (!dispatch(msg, tmpstr.c_str())) {
      return;
    }
  }
}
