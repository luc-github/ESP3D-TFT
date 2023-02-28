/*
  esp3d_gcode_host_service

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
#include "esp3d_gcode_host_types.h"
#include "esp3d_string.h"
#include <list>
#include "esp3d_log.h"
#include "authentication/esp3d_authentication_types.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t id;
    std::string script;
    std::string current_command;
    esp3d_gcode_host_script_type_t type;
    esp3d_gcode_host_state_t state;
    esp3d_gcode_host_wait_t wait_state;
    esp3d_gcode_host_error_t error;
    esp3d_authentication_level_t auth_type;
    uint64_t total;
    uint64_t progress;
    uint64_t timestamp;
} esp3d_script_t;


class Esp3DGCodeHostService : public Esp3DClient
{
public:
    Esp3DGCodeHostService();
    ~Esp3DGCodeHostService();
    bool begin();
    void handle();
    void end();
    void process(esp3d_msg_t * msg);
    bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
    void flush();
    bool started()
    {
        return _started;
    }
    bool processScript(const char * script, esp3d_authentication_level_t auth_type );
    bool abort();
    bool pause();
    bool resume();
private:
    bool isAck(const char *cmd );
    bool isCommand();
    bool isAckNeeded();
    bool startStream();
    bool processCommand();
    bool readNextCommand();
    bool endStream();
    bool isEndChar(uint8_t ch);
    TaskHandle_t _xHandle;
    bool _started;
    pthread_mutex_t _tx_mutex;
    pthread_mutex_t _rx_mutex;
    std::list<esp3d_script_t> _scripts;
    esp3d_script_t * _current_script;


};

extern Esp3DGCodeHostService gcodeHostService;


#ifdef __cplusplus
} // extern "C"
#endif