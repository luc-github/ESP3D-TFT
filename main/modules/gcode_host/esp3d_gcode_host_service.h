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
#include <pthread.h>
#include <stdio.h>

#include <list>

#include "authentication/esp3d_authentication_types.h"
#include "esp3d_client.h"
#include "esp3d_gcode_host_types.h"
#include "esp3d_log.h"
#include "esp3d_string.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Esp3dScript {
  uint8_t id = 0;
  std::string script;
  std::string current_command;
  Esp3dGcodeHostScriptType type = Esp3dGcodeHostScriptType::unknown;
  Esp3dGcodeHostState state = Esp3dGcodeHostState::no_stream;
  Esp3dGcodeHostState next_state = Esp3dGcodeHostState::no_stream;
  Esp3dGcodeHostWait wait_state = Esp3dGcodeHostWait::no_wait;
  Esp3dGcodeHostError error = Esp3dGcodeHostError::no_error;
  Esp3dAuthenticationLevel auth_type = Esp3dAuthenticationLevel::guest;
  uint64_t total = 0;
  uint64_t progress = 0;
  uint64_t timestamp = 0;
  FILE* fileScript = nullptr;
};

class Esp3DGCodeHostService : public Esp3DClient {
 public:
  Esp3DGCodeHostService();
  ~Esp3DGCodeHostService();
  bool begin();
  void handle();
  void end();
  void process(esp3d_msg_t* msg);
  bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
  void flush();
  bool started() { return _started; }
  bool processScript(const char* script, Esp3dAuthenticationLevel auth_type);
  bool abort();
  bool pause();
  bool resume();
  Esp3dGcodeHostState getState();
  Esp3dGcodeHostError getErrorNum();
  Esp3dScript* getCurrentScript();

 private:
  bool isAck(const char* cmd);
  bool isCommand();
  bool isAckNeeded();
  bool startStream();
  bool processCommand();
  bool readNextCommand();
  bool endStream();
  bool isEndChar(uint8_t ch);
  Esp3dGcodeHostScriptType getScriptType(const char* script);
  TaskHandle_t _xHandle;
  bool _started;
  pthread_mutex_t _tx_mutex;
  pthread_mutex_t _rx_mutex;
  std::list<Esp3dScript> _scripts;
  Esp3dScript* _current_script;
};

extern Esp3DGCodeHostService gcodeHostService;

#ifdef __cplusplus
}  // extern "C"
#endif