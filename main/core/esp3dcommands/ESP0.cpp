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

const char * help[]= {"[ESP] (id) - display this help",
                      "[ESP420]display ESP3D current status in plain/JSON",
                      "[ESP444](state) - set ESP3D state (RESET/RESTART)"
                     };

const uint cmdlist[]= {0,
                       420,
                       444,
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
    const uint cmdlistNb = sizeof(cmdlist)/sizeof(char*);
    bool json=hasTag(msg,cmd_params_pos,"json");
    if (cmdNb!=cmdlistNb) {
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
            if(!dispatch(tmpstr.c_str(),target, requestId)) {
                esp3d_log_e("Error sending answer to clients");
                return ;
            }
        }

        if (json) {
            if(!dispatch("]}",target, requestId)) {
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
        if(!dispatch(msg,tmpstr.c_str())) {
            return ;
        }
    }

}