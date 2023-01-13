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
#include "esp3d_settings.h"

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
    bool dispatchAnswer(esp3d_msg_t * msg, uint cmdid, bool json, bool hasError, const char* answerMsg);
    bool dispatchIdValue(bool json,const char *Id, const char * value, esp3d_clients_t target, esp3d_request_t requestId, bool isFirst = false);
    bool dispatchKeyValue(bool json,const char *key, const char * value, esp3d_clients_t target, esp3d_request_t requestId, bool nested=false, bool isFirst=false);
    bool dispatchSetting(bool json,const char * filter, esp3d_setting_index_t index, const char* help, const char ** optionValues, const char ** optionLabels, uint32_t maxsize, uint32_t minsize, uint32_t minsize2,uint8_t precision, const char * unit,  bool needRestart,esp3d_clients_t target, esp3d_request_t requestId, bool isFirst = false);
    bool dispatch(esp3d_msg_t * msg);
    bool dispatch(esp3d_msg_t * msg,uint8_t * sbuf, size_t len);
    bool dispatch(esp3d_msg_t * msg,const char * sbuf);
    bool dispatch(const char * sbuf,  esp3d_clients_t target, esp3d_request_t requestId, esp3d_msg_type_t type=msg_head, esp3d_clients_t origin = ESP3D_COMMAND, esp3d_authentication_level_t authentication_level=ESP3D_LEVEL_GUEST);
    bool dispatchAuthenticationError(esp3d_msg_t * msg, uint cmdid, bool json);
    bool formatCommand(char * cmd, size_t len);
    void ESP0(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP100(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP101(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP102(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP103(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP104(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP105(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP106(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP107(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP108(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP110(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP111(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP112(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP114(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP115(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP120(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP121(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP130(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP131(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP160(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP200(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP202(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP400(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP401(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP402(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP410(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP420(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP444(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP450(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP600(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP610(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP710(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP720(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP730(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP740(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP750(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP780(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP790(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP800(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP900(int cmd_params_pos,esp3d_msg_t * msg);
    void ESP901(int cmd_params_pos,esp3d_msg_t * msg);
    const char * get_param (esp3d_msg_t * msg, uint start,const char* label);
    const char * get_clean_param (esp3d_msg_t * msg, uint start);
    bool has_param (esp3d_msg_t * msg, uint start);
    bool hasTag (esp3d_msg_t * msg, uint start,const char* label);
    void flush();
private:

};

extern Esp3DCommands esp3dCommands;

#ifdef __cplusplus
} // extern "C"
#endif