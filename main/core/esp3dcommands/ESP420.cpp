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

void Esp3DCommands::ESP420(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;

    std::string res = get_clean_param ( msg,cmd_params_pos);
    esp3d_log("Clean *%s*",res.c_str());
    res = get_param ( msg,cmd_params_pos,"json=");
    esp3d_log("Got *%s*",res.c_str());
    if ( hasTag (msg,cmd_params_pos,"json")) {
        esp3d_log("Tag found");
    } else {
        esp3d_log("No tag found");
    }
    if(!dispatch(msg,"420")) {
        Esp3DClient::deleteMsg(msg);
        return ;
    }
    //now need new msg
    esp3d_msg_t * newMsg = Esp3DClient::newMsg(ESP3D_COMMAND, target);
    if (newMsg) {
        if(!dispatch(newMsg,"another line")) {
            Esp3DClient::deleteMsg(newMsg);
            return;
        }
    }

}