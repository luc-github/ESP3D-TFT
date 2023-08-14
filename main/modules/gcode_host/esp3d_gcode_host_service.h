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
#define isEndLineEndFile(ch) \
  ((char)ch == '\n' || (char)ch == '\r' || (char)ch == 0 || (char)ch == -1)
#define isWhiteSpace(ch)                                       \
  ((char)ch == '\n' || (char)ch == '\r' || (char)ch == '\t' || \
   (char)ch == '\v' || (char)ch == '\f' || (char)ch == ' ')
#define isFileStream(stream)                                                 \
  ((((ESP3DGcodeStream*)stream)->type ==                                     \
    ESP3DGcodeHostFileType::filesystem) ||                                   \
   (((ESP3DGcodeStream*)stream)->type == ESP3DGcodeHostFileType::sd_card) || \
   (((ESP3DGcodeStream*)stream)->type == ESP3DGcodeHostFileType::script) ||  \
   (((ESP3DGcodeStream*)stream)->type == ESP3DGcodeHostFileType::sd_script))
#define isCommandStream(stream)                \
  ((((ESP3DGcodeStream*)stream)->type ==       \
    ESP3DGcodeHostFileType::single_command) || \
   (((ESP3DGcodeStream*)stream)->type ==       \
    ESP3DGcodeHostFileType::multiple_commands))

#define ESP_HOST_OK_TIMEOUT 60000
#define ESP_HOST_BUSY_TIMEOUT 10000
#define RING_BUFFER_LENGTH 256

#define ESP_GCODE_HOST_COMMAND_LINE_BUFFER 96  // matches marlin MAX_CMD_SIZE

#ifdef __cplusplus
extern "C" {
#endif

struct ESP3DGcodeStream {
  uint64_t id = 0;  // this is currently the time the stream is created in
                    // millis
  uint64_t totalSize = 0;      // this will correspond to the end file index+1
  uint64_t processedSize = 0;  // this will correspond to the position in a file
  uint64_t lineStart =
      0;  // index of the first character on the line to send. used for
          // switching between streams as new streams are added.
  ESP3DGcodeHostFileType type =
      ESP3DGcodeHostFileType::unknown;  // The type of stream
  ESP3DGcodeStreamState state =
      ESP3DGcodeStreamState::undefined;  // the state of the stream
  ESP3DAuthenticationLevel auth_type =
      ESP3DAuthenticationLevel::guest;  // the authentication level of the user
                                        // requesting the stream
};

struct ESP3DGcodeCommandStream : public ESP3DGcodeStream {
  ESP3DGcodeCommandStream(ESP3DGcodeStream& A) : ESP3DGcodeStream(A) {}
  ESP3DGcodeCommandStream() = default;

  uint32_t bufferPos = 0;
  uint8_t* commandBuffer = nullptr;
};

struct ESP3DGcodeFileStream : public ESP3DGcodeStream {
  ESP3DGcodeFileStream(ESP3DGcodeStream& A) : ESP3DGcodeStream(A) {}
  ESP3DGcodeFileStream() = default;

  FILE* streamFile =
      nullptr;  // pointer to the file to be streamed. Files could be opened and
                // closed whilst in use if it's helpful
  uint64_t commandNumber =
      0;  // Next command number to send. //This should be a gcodehost variable,
          // as only one print stream is possible
  uint64_t resendCommandNumber = 0;  // Requested command to resend.
  uint8_t* _fileName = nullptr;      // this could be a flexible array too.
};

class ESP3DGCodeHostService : public ESP3DClient {
 public:
  ESP3DGCodeHostService();
  ~ESP3DGCodeHostService();
  bool begin();
  void end();
  void handle();
  void process(ESP3DMessage* msg);
  void flush();
  bool started() { return _started; }

  bool newStream(const char* command, size_t length,
                 ESP3DAuthenticationLevel authentication_level,
                 bool isPrintStream = false);
  void updateScripts();
  bool abort();
  bool pause();
  bool resume();
  ESP3DGcodeHostState getState();
  ESP3DGcodeHostError getErrorNum();
  ESP3DGcodeStream* getCurrentStream(
      ESP3DGcodeHostFileType type = ESP3DGcodeHostFileType::active);

 private:
  bool _streamFile(const char* file, ESP3DAuthenticationLevel auth_type,
                   bool executeAsMacro = false, bool executeFirst = false);
  uint8_t _currentCommand[ESP_GCODE_HOST_COMMAND_LINE_BUFFER];
  char _ringBuffer[RING_BUFFER_LENGTH];  // maybe change this to uint8_t
  ESP3DGcodeFileStream* _bufferedStream = nullptr;
  uint32_t _bufferSeam =
      0;  // the position of the earliest byte in the ring buffer
  uint32_t _bufferPos = 0;  // the position of the next byte to be read from the
                            // ring buffer (becomes start on next update)
  bool _bufferFresh =
      true;  // Flag to know if start == pos because the buffer is new or
             // finished. -- should rename to _bufferFull
  bool _isAck(const char* cmd);
  bool _isBusy(const char* cmd);
  bool _isError(const char* cmd);
  uint64_t _resendCommandNumber(const char* cmd);
  bool _isEmergencyParse(const char* cmd);

  bool _isAckNeeded() { return _awaitingAck; };
  bool _openFile(ESP3DGcodeFileStream* stream);
  bool _closeFile(ESP3DGcodeFileStream* stream);
  bool _startStream(ESP3DGcodeStream* stream);
  bool _setStream();

  bool _updateRingBuffer(ESP3DGcodeFileStream* stream);
  char _readCharFromRingBuffer(ESP3DGcodeFileStream* stream);
  char _readCharFromCommandBuffer(ESP3DGcodeCommandStream* stream);

  bool _readNextCommand(ESP3DGcodeStream* stream);
  uint8_t _Checksum(const char* command, uint32_t commandSize);
  bool _CheckSumCommand(char* command, uint32_t commandnb);

  bool _gotoLine(uint64_t line);
  bool _processRx(ESP3DMessage* rx);
  bool _parseResponse(ESP3DMessage* rx);
  bool _endStream(ESP3DGcodeStream* stream);
  ESP3DGcodeHostFileType _getStreamType(const char* file);

  TaskHandle_t _xHandle;
  bool _started;
  const UBaseType_t _xPauseNotifyIndex = 1;
  const UBaseType_t _xResumeNotifyIndex = 2;
  const UBaseType_t _xAbortNotifyIndex = 3;

  bool _awaitingAck = false;
  uint64_t _startTimeout;
  uint64_t _timeoutInterval = ESP_HOST_OK_TIMEOUT;
  ESP3DGcodeHostState _current_state = ESP3DGcodeHostState::idle;
  ESP3DGcodeHostState _next_state = ESP3DGcodeHostState::idle;
  std::list<ESP3DGcodeStream*> _scripts;
  ESP3DGcodeStream* _current_stream = nullptr;
  ESP3DGcodeFileStream _currentPrintStream;
  std::string _stop_script;
  std::string _pause_script;
  std::string _resume_script;

  pthread_mutex_t _tx_mutex;
  pthread_mutex_t _rx_mutex;
};

extern ESP3DGCodeHostService gcodeHostService;

#ifdef __cplusplus
}  // extern "C"
#endif