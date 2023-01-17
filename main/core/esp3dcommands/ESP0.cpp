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

#include "esp3d_commands.h"
#include "esp3d_client.h"
#include <stdio.h>
#include <string>
#include <cstring>

const char * help[]= {"[ESP](id) - display this help",
                      "[ESP100](SSID) - display/set STA SSID",
                      "[ESP101](Password) (CLEAR)- set STA password",
                      "[ESP102](Mode) - display/set STA IP mode (DHCP/STATIC)",
                      "[ESP103](IP=xxxx MSK=xxxx GW=xxxx DNS=XXXXX) - display/set STA IP/Mask/GW/DNS",
                      "[ESP104](State) - display/set sta fallback mode which can be CONFIG, BT, OFF",
                      "[ESP105](SSID) - display/set AP SSID",
                      "[ESP106](Password) - set AP password",
                      "[ESP107](IP) - display/set AP IP",
                      "[ESP108](Chanel) - display/set AP chanel",
                      "[ESP110](State) - display/set radio state which can be STA,AP, CONFIG, BT OFF",
                      "[ESP111] - display current IP",
                      "[ESP112](Hostname) - display/set Hostname",
                      "[ESP114](State) - display/set boot Radio state which can be ON, OFF",
                      "[ESP115](State) - display/set immediate Radio state which can be ON, OFF",
                      "[ESP120](State) - display/set HTTP state which can be ON, OFF",
                      "[ESP121](Port) - display/set HTTP port",
                      "[ESP130](State) - display/set Telnet state which can be ON, OFF, CLOSE",
                      "[ESP131](Port) - display/set Telnet port ",
                      "[ESP160](State) - display/set WebSocket state which can be ON, OFF, CLOSE",
                      "[ESP200](RELEASE) (REFRESH)- display/set SD Card Status",
                      "[ESP202](factor) - display / set  SD Card  SD card Speed divider factor (1 2 4 6 8 16 32)",
                      "[ESP400] - display ESP3D settings",
                      "[ESP401]P=(position) T=(type) V=(value) - Set specific setting",
                      "[ESP402](State) - display/set  Check update at boot state which can be ON, OFF",
                      "[ESP410] - display available AP list",
                      "[ESP420] - display ESP3D current status",
                      "[ESP444](state) - set ESP3D state (RESET/RESTART)",
                      "[ESP450]display ESP3D list on network",
                      "[ESP600](message) - send notification",
                      "[ESP610](type=NONE/PUSHOVER/EMAIL/LINE/IFTTT) (AUTO=YES/NO) (T1=token1) (T2=token2) (TS=Settings)",
                      "[ESP710]FORMATFS - Format ESP3D Filesystem",
                      "[ESP720](path) - List ESP3D Filesystem",
                      "[ESP730](Action)=(path) - rmdir / remove / mkdir / exists / create on ESP3D FileSystem (path)",
                      "[ESP740](path) - List SD Filesystem",
                      "[ESP750](Action)=(path) - rmdir / remove / mkdir / exists / create on SD (path)",
                      "[ESP780](path) - List Global Filesystem",
                      "[ESP790](Action)=(path) - rmdir / remove / mkdir / exists / create on Global FileSystem (path)",
                      "[ESP800](time=YYYY-MM-DDTHH:mm:ss)(version= webui version)(setup=0/1) - display FW Informations /set time",
                      "[ESP900](state) - display/set serial state(ENABLE/DISABLE)",
                      "[ESP901](baud rate) - display/set serial baud rate",
#if ESP3D_USB_SERIAL_FEATURE
                      "[ESP902](baud rate) - display/set usb-serial baud rate",
                      "[ESP950]<SERIAL/USB>  - display/set usb-serial client output",
#endif //#if ESP3D_USB_SERIAL_FEATURE
                     };

const uint cmdlist[]= {0,
                       100,
                       101,
                       102,
                       103,
                       104,
                       105,
                       106,
                       107,
                       108,
                       110,
                       111,
                       112,
                       114,
                       115,
                       120,
                       121,
                       130,
                       131,
                       160,
                       200,
                       202,
                       400,
                       401,
                       402,
                       410,
                       420,
                       450,
                       444,
                       600,
                       610,
                       710,
                       720,
                       730,
                       740,
                       750,
                       780,
                       790,
                       800,
                       900,
                       901,
#if ESP3D_USB_SERIAL_FEATURE
                       902,
                       950,
#endif //#if ESP3D_USB_SERIAL_FEATURE
                      };
//ESP3D Help
//[ESP0] or [ESP]<command>
void Esp3DCommands::ESP0(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    std::string tmpstr;
    const uint cmdNb = sizeof(help)/sizeof(char*);
    const uint cmdlistNb = sizeof(cmdlist)/sizeof(uint);
    bool json=hasTag(msg,cmd_params_pos,"json");
    if (cmdNb!=cmdlistNb) {
        esp3d_log("Help corrupted: %d vs %d",cmdNb,cmdlistNb);
        msg->type = msg_unique;
        if(!dispatch(msg,"Help corrupted")) {
            esp3d_log_e("Error sending command to clients");
        }
        return ;
    }
    tmpstr = get_clean_param(msg,cmd_params_pos);
    if (tmpstr.length()==0) {
        //use msg for first answer
        if (json) {
            tmpstr = "{\"cmd\":\"0\",\"status\":\"ok\",\"data\":[";
        } else {
            tmpstr = "[List of ESP3D commands]\n";

        }
        msg->type = msg_head;
        if(!dispatch(msg,tmpstr.c_str())) {
            esp3d_log_e("Error sending command to clients");
            return ;
        }
        for (uint i = 0; i < cmdNb; i++) {
            if (json) {
                tmpstr = "{\"id\":\"";
                tmpstr +=std::to_string(cmdlist[i]);
                tmpstr +="\",\"help\":\"";
                tmpstr +=help[i];
                tmpstr +="\"}";
                if (i < cmdNb - 1 && (cmdNb - 1)>0) {
                    tmpstr +=",";
                }
            } else {
                tmpstr = help[i];
                tmpstr+="\n";
            }
            if(!dispatch(tmpstr.c_str(),target, requestId, msg_core)) {
                esp3d_log_e("Error sending answer to clients");
                return ;
            }
        }

        if (json) {
            if(!dispatch("]}",target, requestId, msg_tail)) {
                esp3d_log_e("Error sending answer to clients");
                return ;
            }
        } else {
            if(!dispatch("ok\n",target, requestId, msg_tail)) {
                esp3d_log_e("Error sending answer to clients");
                return ;
            }
        }
    } else {
        uint cmdval = atoi(tmpstr.c_str());
        for (uint i = 0; i < cmdNb; i++) {
            if (cmdlist[i] == cmdval) {
                if (json) {
                    tmpstr = "{\"cmd\":\"0\",\"status\":\"ok\",\"data\":{\"id\":\"";
                    tmpstr +=std::to_string(cmdval);
                    tmpstr +="\",\"help\":\"";
                    tmpstr +=help[i];
                    tmpstr +="\"}}";
                } else {
                    tmpstr = help[i];
                    tmpstr+="\n";
                }
                msg->type = msg_unique;
                if(!dispatch(msg,tmpstr.c_str())) {
                    return ;
                }
                return;
            }
        }
        if (json) {
            tmpstr = "{\"cmd\":\"0\",\"status\":\"error\",\"data\":\"This command is not supported: ";
            tmpstr +=std::to_string(cmdval);
            tmpstr +="\"}";
        } else {
            tmpstr = "This command is not supported: ";
            tmpstr +=std::to_string(cmdval);
            tmpstr+="\n";
        }
        msg->type = msg_unique;
        if(!dispatch(msg,tmpstr.c_str())) {
            return ;
        }
    }

}