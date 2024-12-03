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
#include "tasks_def.h"
#define ESP3D_MAX_STREAM_SIZE 50

#ifdef __cplusplus
extern "C" {
#endif

struct ESP3DGcodeStream {
  uint64_t id = 0;  // this is currently the time the stream is created in
                    // millis
  uint64_t totalSize =
      0;  // this will correspond to file size or commands line length
  uint64_t processedSize =
      0;  // this will correspond to the position in a file for next read
  uint64_t cursorPos =
      0;  // index of the first character on the line to send. used for
          // switching between streams as new streams are added.
  ESP3DGcodeHostStreamType type =
      ESP3DGcodeHostStreamType::unknown;  // The type of stream
  ESP3DGcodeStreamState state =
      ESP3DGcodeStreamState::undefined;  // the state of the stream
  ESP3DAuthenticationLevel auth_type =
      ESP3DAuthenticationLevel::guest;  // the authentication level of the user
                                        // requesting the stream
  bool active = false;      // is the stream currently being processed
  char *dataStream = NULL;  // the name of the file to stream
};

class ESP3DGCodeHostService : public ESP3DClient {
 public:
  ESP3DGCodeHostService();
  ~ESP3DGCodeHostService();
  bool begin();
  void end();
  void handle();
  void process(ESP3DMessage *msg);
  void flush();
  bool started() { return _started; }

  void updateScripts();
  bool abort();
  bool pause();
  bool resume();

  ESP3DGcodeHostState getState();

  ESP3DGcodeHostError getErrorNum();
  ESP3DGcodeStream *getCurrentMainStream();
  bool addStream(const char *filename, ESP3DAuthenticationLevel auth_type,
                 bool executeAsMacro);
  bool addStream(const char *command, size_t length,
                 ESP3DAuthenticationLevel authentication_level);

  size_t getScriptsListSize() { return _scripts.size(); }
  size_t getStreamsListSize() { return _scripts.size(); }
  bool hasStreamListCommand(const char *command);

 private:
  ESP3DGcodeHostStreamType _getStreamType(const char *data);
  ESP3DGcodeStream *_get_front_script();
  ESP3DGcodeStreamState _getStreamState();
  bool _setStreamState(ESP3DGcodeStreamState state);
  bool _setMainStreamState(ESP3DGcodeStreamState state);
  bool _setStreamRequestState(ESP3DGcodeStreamState state);
  bool _startStream(ESP3DGcodeStream *stream);
  bool _endStream(ESP3DGcodeStream *stream);
  bool _openFile(ESP3DGcodeStream *stream);
  bool _closeFile(ESP3DGcodeStream *stream);
  bool _pushBackGCodeStream(ESP3DGcodeStream *stream, bool is_stream = false);
  bool _popFrontGCodeStream(bool is_stream = false);

  void _handle_notifications();
  void _handle_msgs();
  void _handle_stream_selection();
  void _handle_stream_states();
  bool _add_stream(const char *data, ESP3DAuthenticationLevel auth_type,
                   bool executeFirst = false);

  bool _readNextCommand(ESP3DGcodeStream *stream);
  uint8_t _Checksum(const char *command, uint32_t commandSize);
  bool _CheckSumCommand(char *result_buffer, size_t max_result_size,
                        const char *command, uint32_t commandnb);
  bool _stripCommand();

  bool _processRx(ESP3DMessage *rx);
  bool _parseResponse(ESP3DMessage *rx);

  std::string _current_command_str;
  size_t _file_buffer_length = 0;
  char _file_buffer[STREAM_CHUNK_SIZE];

  TaskHandle_t _xHandle = NULL;
  bool _started = false;
  const UBaseType_t _xPauseNotifyIndex = 0;
  const UBaseType_t _xResumeNotifyIndex = 1;
  const UBaseType_t _xAbortNotifyIndex = 2;

  ESP3DClientType _outputClient = ESP3DClientType::no_client;
  bool _awaitingAck = false;
  uint64_t _startTimeout = 0;

  ESP3DGcodeStreamState _requested_state = ESP3DGcodeStreamState::undefined;
  std::list<ESP3DGcodeStream *> _scripts;
  std::list<ESP3DGcodeStream *> _streams;
  ESP3DGcodeStream *_current_stream_ptr = nullptr;
  ESP3DGcodeStream *_current_main_stream_ptr = nullptr;

  std::string _stop_script;
  std::string _pause_script;
  std::string _resume_script;
  bool _connection_lost = false;

  ESP3DGcodeHostError _error = ESP3DGcodeHostError::no_error;

  // Stream Variables:
  FILE *_file_handle =
      nullptr;  // pointer to the file to be streamed. Files could be
                // opened and closed whilst in use if it's helpful
  uint64_t _command_number =
      0;  // Next command number to send. //This should be a gcodehost variable,
          // as only one print stream is possible
  uint64_t _resend_command_number = 0;  // Requested command to resend.
  uint8_t _resend_command_counter =
      0;  // Number of times the resend command has been requested
  pthread_mutex_t _tx_mutex;
  pthread_mutex_t _rx_mutex;
  pthread_mutex_t _streams_list_mutex;
  pthread_mutex_t _scripts_list_mutex;
};

extern ESP3DGCodeHostService gcodeHostService;

#ifdef __cplusplus
}  // extern "C"
#endif