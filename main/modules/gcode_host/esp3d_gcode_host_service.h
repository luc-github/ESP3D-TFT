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


#define isEndLine(ch) ((char)ch == '\n' || (char)ch == '\r')
#define isEndLineEndFile(ch) ((char)ch == '\n' || (char)ch == '\r' || (char)ch == 0 || (char)ch == -1)
#define isWhiteSpace(ch) ((char)ch =='\n' || (char)ch =='\r' || (char)ch == ' ')
#define isFileStream(stream) ((((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::filesystem) || (((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::sd_card) || (((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::script) || (((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::sd_script))
#define isCommandStream(stream) ((((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::single_command) || (((ESP3DGcodeStream*)stream) -> type == ESP3DGcodeHostFileType::multiple_commands))

#define ESP_HOST_OK_TIMEOUT 60000
#define ESP_HOST_BUSY_TIMEOUT 10000
#define FILE_BUFFER_LENGTH 256

#define ESP_GCODEHOST_COMMAND_LINE_BUFFER 96 //matches marlin MAX_CMD_SIZE

#ifdef __cplusplus
extern "C" {
#endif

struct ESP3DGcodeStream{
  uint64_t id = 0; //this is currently the time the stream is created in millis
  uint64_t totalSize = 0; //this will correspond to the end file index+1
  uint64_t processedSize = 0; //this will correspond to the position in a file
  // need some kind of var for the buffer location
  //char* bufferPos = nullptr; // probably better to just use an integer tbh
  uint32_t bufferPos = 0;
  char currentCommand[ESP_GCODEHOST_COMMAND_LINE_BUFFER];
  ESP3DGcodeHostFileType type = ESP3DGcodeHostFileType::unknown; //The type of stream
  ESP3DGcodeStreamState state = ESP3DGcodeStreamState::start; //the state for handle to perform - maybe rename to step
  //ESP3DGcodeStreamState next_state = ESP3DGcodeStreamState::read_line; //the next action to perform after command is dispatched 
  ESP3DAuthenticationLevel auth_type = ESP3DAuthenticationLevel::guest; //the authentication level of the user requesting the stream
};

struct ESP3DGcodeCommandStream : public ESP3DGcodeStream
{
  ESP3DGcodeCommandStream(const ESP3DGcodeStream & A) : ESP3DGcodeStream(A){}
  ESP3DGcodeCommandStream() = default;

  char* commandBuffer = nullptr; //the buffer for command strings (terminal commands, non file macros)
};

struct ESP3DGcodeFileStream : public ESP3DGcodeStream
{
  ESP3DGcodeFileStream(const ESP3DGcodeStream & A) : ESP3DGcodeStream(A){}
  ESP3DGcodeFileStream() = default;

  char* _fileName = nullptr;
  FILE* streamFile = nullptr;
  //char* fileBuffer = nullptr;
  char fileBuffer[FILE_BUFFER_LENGTH];
  int chunksize = FILE_BUFFER_LENGTH;
  uint64_t commandNumber =  0;
  uint64_t resendCommandNumber = 0;
};

/*

struct ESP3DScript {
  uint64_t id = 0;
  std::string script;
  std::string current_command;
  ESP3DGcodeHostScriptType type = ESP3DGcodeHostScriptType::unknown;
  ESP3DGcodeHostState state = ESP3DGcodeHostState::start;
  ESP3DGcodeHostState next_state = ESP3DGcodeHostState::undefined;
  ESP3DGcodeHostWait wait_state = ESP3DGcodeHostWait::no_wait;
  ESP3DGcodeHostError error = ESP3DGcodeHostError::no_error;
  ESP3DAuthenticationLevel auth_type = ESP3DAuthenticationLevel::guest;
  uint64_t total = 0;
  uint64_t progress = 0;
  uint64_t timestamp = 0;
  FILE* fileScript = nullptr;
};

*/

class ESP3DGCodeHostService : public ESP3DClient {
 public:
  ESP3DGCodeHostService();
  ~ESP3DGCodeHostService();
  bool begin();
  void end();
  void _handle();
  void process(ESP3DMessage* msg); //replace with _processCommand() or similar
  bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
  void flush();
  bool started() { return _started; }
  bool stream(const char* file, ESP3DAuthenticationLevel auth_type, bool executeAsMacro = false, bool executeFirst = false);
  bool abort();
  bool pause();
  bool resume();
  ESP3DGcodeHostState getState();
  ESP3DGcodeHostError getErrorNum();
  ESP3DGcodeStream* getCurrentStream(
      ESP3DGcodeHostFileType type = ESP3DGcodeHostFileType::active);

 private:

  bool _isAck(const char* cmd);
  bool _isBusy(const char* cmd);
  bool _isError(const char* cmd);
  uint64_t _resendCommandNumber(const char* cmd);
  bool _isEmergencyParse(const char* cmd);
  //bool _isAckNeeded();
  bool _startStream(ESP3DGcodeStream* _stream);
  bool _readNextCommand(ESP3DGcodeStream* _stream);
  bool _gotoLine(uint64_t line);
  bool _processRx(ESP3DMessage* rx);
  bool _openFile(ESP3DGcodeStream* _stream);
  bool _parseResponse(ESP3DMessage* rx);
  bool _endStream(ESP3DGcodeStream* _stream);
  bool _isEndChar(uint8_t ch);
  bool _isWhiteSpace(uint8_t ch);
  ESP3DGcodeHostFileType _getStreamType(const char* file);

  TaskHandle_t _xHandle;
  bool _started;
  const UBaseType_t xPauseIndex = 1;
  const UBaseType_t xResumeIndex = 2;
  const UBaseType_t xAbortIndex = 3;
  
  bool _awaitingAck = false;
  uint64_t _startTimeout;
  uint64_t _timeoutInterval = ESP_HOST_OK_TIMEOUT;
  ESP3DGcodeHostState state = ESP3DGcodeHostState::wait;
  ESP3DGcodeHostState next_state = ESP3DGcodeHostState::wait;
  std::list<ESP3DGcodeStream> _scripts; // NO LONGER TRUE -----> Print file streams (filesystem, sd_card types) will be added to the front of this list. Only one will be allowed. All other types are macros/terminal commands will be added to the back and executed in order.
  ESP3DGcodeStream* _current_stream;
  ESP3DGcodeFileStream _currentPrintStream;
};

extern ESP3DGCodeHostService gcodeHostService;

#ifdef __cplusplus
}  // extern "C"
#endif