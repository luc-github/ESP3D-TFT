/*
  esp3d_commands

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

#pragma once

#include <stdio.h>
#include "esp3d_client.h"
#include "authentication/esp3d_authentication.h"

#ifdef __cplusplus
extern "C" {
#endif


class Esp3DCommands
{
public:
    Esp3DCommands();
    ~Esp3DCommands();
    bool is_esp_command(uint8_t * sbuf, size_t len);
    void process(esp3d_msg_t * msg);
    void execute_internal_command(int cmd, int cmd_params_pos,esp3d_msg_t * msg);
    bool dispatch(esp3d_msg_t * msg);
    bool dispatch(esp3d_msg_t * msg,uint8_t * sbuf, size_t len);
    bool dispatch(esp3d_msg_t * msg,const char * sbuf);
    bool dispatch(const char * sbuf,  esp3d_clients_t target, esp3d_clients_t origin = ESP3D_COMMAND, esp3d_authentication_level_t authentication_level=ESP3D_LEVEL_GUEST);
    void ESP0(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP420(int cmd_params_pos,esp3d_msg_t * msg);
    const char * get_param (esp3d_msg_t * msg, uint start,const char* label);
    const char * get_clean_param (esp3d_msg_t * msg, uint start);
    bool hasTag (esp3d_msg_t * msg, uint start,const char* label);
private:

};

extern Esp3DCommands esp3dCommands;

#ifdef __cplusplus
} // extern "C"
#endif