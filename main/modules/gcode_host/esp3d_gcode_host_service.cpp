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
#include "esp3d_gcode_host_service.h"

#include <stdio.h>

#include "serial/esp3d_serial_client.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // #if ESP3D_USB_SERIAL_FEATURE

#include "driver/uart.h"
#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "filesystem/esp3d_flash.h"
#include "filesystem/esp3d_globalfs.h"
#include "serial_def.h"

#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

#include "esp32/rom/crc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks_def.h"

// Macro behaviour:
// to be executed regardless of streaming state (ie paused)
// to be executed before resuming with file stream

ESP3DGCodeHostService gcodeHostService;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout

// main task
static void esp3d_gcode_host_task(void* pvParameter) {
  (void)pvParameter;
  gcodeHostService.updateScripts();
  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));
    gcodeHostService.handle();
  }
  vTaskDelete(NULL);
}

/// @brief Ascertains the type of stream referred to by a string.
/// @param file Const Char Pointer, to an array containing a command string, or
/// a file system path.
/// @return Returns a stream type, either FS, SD, single or multiple commands.
ESP3DGcodeHostFileType ESP3DGCodeHostService::_getStreamType(
    const char* data) {  // maybe this can also check for invalid file
                         // extensions if not done elsewhere
  if (strlen(data) == 0) {
    return ESP3DGcodeHostFileType::invalid;
  }
  if (data[0] == '/') {
    ESP3DGcodeHostFileType type = ESP3DGcodeHostFileType::fs_stream;
    if (strstr(data, "/sd/") == data) {
#if ESP3D_SD_CARD_FEATURE
      type = ESP3DGcodeHostFileType::sd_stream;

#endif  // ESP3D_SD_CARD_FEATURE
    }
    return type;
  } else {
    if (strstr(data, "\n") != nullptr || (strstr(data, ";") != nullptr)) {
      return ESP3DGcodeHostFileType::multiple_commands;
    } else {
      return ESP3DGcodeHostFileType::single_command;
    }
  }
}

// Update scripts from settings
void ESP3DGCodeHostService::updateScripts() {
  char buffer[SIZE_OF_SCRIPT + 1];
  _stop_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_stop_script, buffer, SIZE_OF_SCRIPT);
  _resume_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_resume_script, buffer, SIZE_OF_SCRIPT);
  _pause_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_pause_script, buffer, SIZE_OF_SCRIPT);
}

// Create a new stream from a command msg
bool ESP3DGCodeHostService::addStream(
    const char* command, size_t length,
    ESP3DAuthenticationLevel authentication_level) {
  // the command has a 0x0 terminal so we should be able to ignore the length
  // because it is msg we should consider it as a command so we can execute it
  // first
  return _streamData(command, authentication_level, true);
}

// Add stream from ESP700 command
// Do we need semaphores here?
// In theory no because each command [ESP700] is processed in a single task and
// each commands are process one after the others
bool ESP3DGCodeHostService::addStream(const char* filename,
                                      ESP3DAuthenticationLevel auth_type,
                                      bool executeAsMacro) {
  ESP3DGcodeHostFileType type = _getStreamType(filename);
  //  ESP700 only accepts file names, not commands
  //  as commands can be sent directly, no need to use ESP700
  if (!isFileStream(type)) return false;
  return _streamData(filename, auth_type, executeAsMacro);
}

bool ESP3DGCodeHostService::_streamData(const char* data,
                                        ESP3DAuthenticationLevel auth_type,
                                        bool executeFirst) {
  esp3d_log("Processing stream request: %s, with authentication level=%d", data,
            static_cast<uint8_t>(auth_type));
  // Macro should be executed first like any other command
  // only file stream is executed after current stream is finished
  ESP3DGcodeHostFileType type = _getStreamType(data);
  esp3d_log("File type: %d", static_cast<uint8_t>(type));
  if (type == ESP3DGcodeHostFileType::invalid) {
    esp3d_log_e("Invalid file type");
    return false;
  }
  // any stream that is first is a macro/script
  if (executeFirst) {
    if (type == ESP3DGcodeHostFileType::fs_stream) {
      type = ESP3DGcodeHostFileType::fs_script;
#if ESP3D_SD_CARD_FEATURE
    } else if (type == ESP3DGcodeHostFileType::sd_stream) {
      type = ESP3DGcodeHostFileType::sd_script;
#endif  // ESP3D_SD_CARD_FEATURE
    }
  }
  // Create a new stream
  ESP3DGcodeStream* new_stream =
      (ESP3DGcodeStream*)malloc(sizeof(ESP3DGcodeStream));
  if (new_stream == nullptr) {
    esp3d_log_e("Failed to allocate memory for new stream");
    return false;
  }
  esp3d_log("New stream type: %d", static_cast<uint8_t>(_current_stream.type));
  new_stream->type = type;
  new_stream->id = esp3d_hal::millis();
  new_stream->auth_type = auth_type;
  new_stream->cursorPos = 0;
  new_stream->processedSize = 0;
  new_stream->totalSize = 0;
  new_stream->state = ESP3DGcodeStreamState::start;
  new_stream->dataStream = data;
  // Sanity check for multiple commands
  if (type == ESP3DGcodeHostFileType::multiple_commands) {
    for (int i = 0; i < strlen(data); i++) {
      if (new_stream->dataStream[i] == ';') {
        new_stream->dataStream[i] = '\n';
      }
    }
  }
  // Add to the list
  if (executeFirst) {
    _scripts.push_front(new_stream);
  } else {
    _scripts.push_back(new_stream);
  }

  // FIXME: this must be done when starting to process a new stream
  // and also need to reset the command number to 0
  // and also need to reset the resend command number to 0
  //  send line number reset command if commencing a print stream.
  //  FIXME: Use gcode parser to get this command
  // const char lineno_reset[] =
  //     "M110 N0\n";  // Is there a better way to store this const?
  //_streamData(lineno_reset, auth_type, true);

  return true;
}

bool ESP3DGCodeHostService::abort() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xAbortNotifyIndex);
  return true;
}

bool ESP3DGCodeHostService::pause() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xPauseNotifyIndex);
  return true;
}

bool ESP3DGCodeHostService::resume() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xResumeNotifyIndex);
  return true;
}

bool ESP3DGCodeHostService::updateOutputClient() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xOutputChangeNotifyIndex);
  return true;
}

// ##################### Parsing Functions ########################
// FIXME: use the GCODE parser to get this command
bool ESP3DGCodeHostService::_isAck(const char* cmd) {
  if ((strstr(cmd, "ok") != nullptr)) {
    esp3d_log("Got ok");
    return true;
  }
  return false;
}

// FIXME: use the GCODE parser to get this command
bool ESP3DGCodeHostService::_isBusy(const char* cmd) {
  if ((strstr(cmd, "busy:") != nullptr)) {
    esp3d_log("Got busy");
    return true;
  }
  return false;
}

// FIXME: use the GCODE parser to get this command
bool ESP3DGCodeHostService::_isError(const char* cmd) {
  return false;  // need info on printer error formats
}

// FIXME: use the GCODE parser to get this command
uint64_t ESP3DGCodeHostService::_resendCommandNumber(
    const char* cmd) {  // we should rename this
  char* p = nullptr;
  if (((p = strstr(cmd, "resend:")) != nullptr)) {
    esp3d_log("Got resend");
    return atoi(p + 7);
  }
  return 0;
}

// FIXME: use the GCODE parser to get this command
bool ESP3DGCodeHostService::_isEmergencyParse(const char* cmd) {
  if ((strstr(cmd, "M112") != nullptr) || (strstr(cmd, "M108") != nullptr)) {
    return true;
  }
  return false;
}

// ##################### File/Script Handling Functions
// ########################

/// @brief Opens the file assosciated with the provided file stream, and seeks
/// to the correct position.
/// @param stream Pointer to the file stream to be opened.
/// @return True if file opened successfully.
bool ESP3DGCodeHostService::_openFile(ESP3DGcodeStream* stream) {
  esp3d_log("File name is %s", stream->dataStream.c_str());
  if (globalFs.accessFS(stream->dataStream.c_str())) {
    if (globalFs.exists(stream->dataStream.c_str())) {
      esp3d_log("File exists");
      _File = globalFs.open(stream->dataStream.c_str(), "r");
      if (_File != nullptr) {
        if (_current_stream_ptr->cursorPos != 0) {
          if (fseek(_File, (long)_current_stream_ptr->cursorPos, SEEK_SET) !=
              0) {  // this would need adjusting if concurrrent file access is
                    // implemented (seek to buffer end instead)
            esp3d_log_e(
                "Failed to seek to correct position in file: %s",
                ((ESP3DGcodeStream*)_current_stream_ptr)->dataStream.c_str());
            return false;
            // ERROR HERE
          }
        }
        esp3d_log("File opened");
        return true;
      } else {
        esp3d_log_e("Failed to open file");
      }
    } else {
      esp3d_log_e("File does not exist");
    }
  } else {
    esp3d_log_e("Can't access FS");
  }
  return false;
}

/// @brief Closes the File assosciated with the provided file stream.
/// @param stream Pointer to the file stream to be closed.
/// @return True if file closed and FS released successfully.
bool ESP3DGCodeHostService::_closeFile(ESP3DGcodeStream* stream) {
  if (_File == nullptr) {
    esp3d_log_e("No file to close");
    return false;
  }

  esp3d_log("Closing File: %s", stream->dataStream.c_str());
  globalFs.close((_File), stream->dataStream.c_str());
  globalFs.releaseFS(stream->dataStream.c_str());

  return true;
}

/// @brief Update the ring buffer with data from the provided file stream.
/// @param stream Pointer to the file stream to update the buffer with.
/// @return True if buffer successfully updated.
bool ESP3DGCodeHostService::_updateRingBuffer(ESP3DGcodeStream* stream) {
  // _bufferSeam: This is the index of the earliest byte (end) in the buffer.

  // if the correct stream is not buffered, reset and fill the buffer
  if (_bufferedStream != stream) {
    // Set indexes and load new data
    _cursorPos = 0;
    _bufferedStream = stream;
    _bufferFresh = true;

    if ((_bufferSeam = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH,
                             _File)) == RING_BUFFER_LENGTH) {
      _bufferSeam = 0;
    } else {
      _ringBuffer[_bufferSeam] = 0;
    }
    esp3d_log("Buffered stream switched");

  } else {
    if ((_cursorPos == _bufferSeam) && (_bufferFresh == true)) {
      // esp3d_log("Buffer is full");
      return false;  // return, buffer is already full of fresh data
    }
    if ((_ringBuffer[_bufferSeam] == 0) || (_ringBuffer[_cursorPos] == 0)) {
      // esp3d_log("File finished");
      return false;  // return, file finished
    }

    esp3d_log("Updating buffer from: %s", _ringBuffer);

    int empty = 0;
    int read = 0;
    // if we've reached the seam we need to read a whole buffer
    if (_cursorPos == _bufferSeam) {
      empty = RING_BUFFER_LENGTH;  // maybe not needed
      read = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH, _File);
      _cursorPos = 0;
      if (read == empty) {
        _bufferSeam = 0;
        _bufferFresh = true;
        esp3d_log("Read full buffer");
      } else {  // if fewer bytes are read than were requested, end of file is
                // in the buffer - place EOF char accordingly.
        _bufferSeam = read;  // 0 indexing makes this the byte after last char.
        _ringBuffer[_bufferSeam] = 0;  // if this is 0, it's the end of file
        esp3d_log("Read partial buffer");
      }

      // if we're after the seam do a single read to update from the seam to
      // our position
    } else if (_cursorPos > _bufferSeam) {
      empty = _cursorPos - _bufferSeam;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char), empty, _File);
      if (read == empty) {
        _bufferSeam = _cursorPos;
        _bufferFresh = true;
      } else {
        while (read > 0) {
          (_bufferSeam == RING_BUFFER_LENGTH - 1) ? _bufferSeam = 0
                                                  : _bufferSeam++;
          read--;
        }
        _ringBuffer[_bufferSeam] = 0;
      }
      esp3d_log("Updated Buffer");
      // if we're before the seam do two reads to update from seam to buffer
      // end, and from buffer start to current position.
    } else {
      empty = RING_BUFFER_LENGTH - _bufferSeam + _cursorPos;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char),
                   RING_BUFFER_LENGTH - _bufferSeam, _File);
      if (_cursorPos > 0) {
        read += fread(_ringBuffer, sizeof(char), _cursorPos, _File);
      }
      if (read == empty) {
        _bufferSeam = _cursorPos;
        _bufferFresh = true;
      } else {
        while (read > 0) {
          (_bufferSeam == RING_BUFFER_LENGTH - 1) ? _bufferSeam = 0
                                                  : _bufferSeam++;
          read--;
        }
        _ringBuffer[_bufferSeam] = 0;
      }
      esp3d_log("Updated Buffer");
    }
  }

  esp3d_log("Updated buffer to: %s", _ringBuffer);
  return true;
}

/// @brief Read a single character from the ring buffer, from the provided
/// file stream. Will update contents of the buffer as required.
/// @param stream Pointer to the file stream to read a character from.
/// @return Current character from the buffer. Will return 0 at the end of the
/// stream.
char ESP3DGCodeHostService::_readCharFromRingBuffer(ESP3DGcodeStream* stream) {
  // if we're at the seam, and not fresh, whole buffer is used and needs
  // updating.
  if (_bufferFresh == true) {
    if (_cursorPos != _bufferSeam) {
      _bufferFresh = false;
    }
  } else if (_cursorPos == _bufferSeam) {
    if (_bufferFresh == false) _updateRingBuffer(stream);
  }

  char c = _ringBuffer[_cursorPos];
  (_cursorPos == RING_BUFFER_LENGTH - 1) ? _cursorPos = 0 : _cursorPos++;
  return c;  // if this returns NULL, we're at endfile
}

/// @brief Read a single character from the buffer of the provided command
/// stream.
/// @param stream Pointer to the command buffer to read a character from.
/// @return Current character from the buffer. Will return 0 at the end of the
/// stream.
char ESP3DGCodeHostService::_readCharFromCommandBuffer(
    ESP3DGcodeStream* stream) {
  char c = stream->dataStream[((ESP3DGcodeStream*)stream)->cursorPos];
  stream->cursorPos++;
  return c;
}

// ##################### Stream Handling Functions ########################

// would be better to eliminate this function, and find the size of the stream
// elsewhere. (stream() Maybe)
bool ESP3DGCodeHostService::_startStream(ESP3DGcodeStream* stream) {
  // function may not be needed after stream() <------ do this for sure.
  // nothing to be done if command stream
  stream->cursorPos = 0;
  if (isFileStream(stream)) {
    if (((ESP3DGcodeStream*)stream)->dataStream != "") {
      int pos =
          ftell(_File) + 1;  // we can get rid of this if we make sure to update
                             // the buffer after this function is called.
      fseek(_File, 0, SEEK_END);
      ((ESP3DGcodeStream*)stream)->totalSize =
          ftell(_File) + 1;  // +1? for size instead of end index
      fseek(_File, pos, SEEK_SET);
      esp3d_log("Size is: %d",
                (unsigned int)((ESP3DGcodeStream*)stream)->totalSize);
      return true;
    }
    esp3d_log_e("Stream file not currently open");
    return false;
  } else if (isCommandStream(stream)) {
    ((ESP3DGcodeStream*)stream)->totalSize =
        ((ESP3DGcodeStream*)stream)->dataStream.size();
    esp3d_log("Size is: %d",
              (unsigned int)((ESP3DGcodeStream*)stream)->totalSize);
    return true;
  }
  esp3d_log_e("Unknown stream type");
  return false;
}

/// @brief End the current stream, freeing any allocated memory and removing
/// scripts from the queue.
/// @param stream Pointer to the stream to end and free.
/// @return Always true.
bool ESP3DGCodeHostService::_endStream(ESP3DGcodeStream* stream) {
  esp3d_log("Freeing stream");
  for (auto streamPtr = _scripts.begin(); streamPtr != _scripts.end();
       ++streamPtr) {
    if ((*streamPtr)->id == stream->id) {
      esp3d_log("Clear session %s succeed", sessionId);
      if (isFileStream(stream)) {
        _closeFile(stream);  // Behaviour may need altering depending
      }
      free(stream);
      _scripts.erase(streamPtr++);
      return true;
    }
  }
  esp3d_log_e("Stream not found");
  return false;
}

/// @brief Read the next command line from the provided stream into
/// _currentCommand. Skips past comments and empty lines.
/// @param stream Pointer to the command or file stream to be read from.
/// @return True if command is read, False if no command read (end of stream).
bool ESP3DGCodeHostService::_readNextCommand(ESP3DGcodeStream* stream) {
  stream->cursorPos = stream->processedSize;
  uint32_t cmdPos = 0;

  if (isFileStream(stream)) {
    char c = (char)0;

    do {
      c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
      ((ESP3DGcodeStream*)stream)->processedSize++;

      while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
        c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
        ((ESP3DGcodeStream*)stream)->processedSize++;
      }
      while (c == ';') {  // while its a full line comment, skim to next line
        while (!(isEndLineEndFile(c))) {
          c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
          ((ESP3DGcodeStream*)stream)->processedSize++;
        }
        while (isWhiteSpace(c)) {  // ignore leading spaces and empty lines
          c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
          ((ESP3DGcodeStream*)stream)->processedSize++;
        }
      }
      stream->cursorPos = stream->processedSize;

      while (!(isEndLineEndFile(c) ||
               (((ESP3DGcodeStream*)stream)->processedSize >=
                ((ESP3DGcodeStream*)stream)
                    ->totalSize))) {  // while not end of line or end of file
        if (c == ';') {               // reached a comment, skim to next line
          while (!(isEndLineEndFile(c))) {
            c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
            ((ESP3DGcodeStream*)stream)->processedSize++;
          }
        } else {  // no comment yet, read into command
          _currentCommand[cmdPos] = c;
          cmdPos++;
          c = _readCharFromRingBuffer((ESP3DGcodeStream*)stream);
          ((ESP3DGcodeStream*)stream)->processedSize++;
        }
      }

    } while (!isEndLineEndFile(c));

  } else if (isCommandStream(stream)) {
    esp3d_log("Is command stream");
    esp3d_log("Buffer size is: %d",
              ((ESP3DGcodeStream*)stream)->dataStream.size());
    char c = (char)0;
    if (((ESP3DGcodeStream*)stream)->cursorPos >=
        stream->totalSize) {  // if we reach the end of the buffer, the
                              // command stream is done
      esp3d_log("End of Buffer");
      return false;
    }

    c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
    ((ESP3DGcodeStream*)stream)->processedSize++;
    while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
      esp3d_log("Is Whitespace");
      c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
      ((ESP3DGcodeStream*)stream)->processedSize++;
    }
    while (c ==
           ';') {  // while its a full line comment, read on to the next line
      c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
      ((ESP3DGcodeStream*)stream)->processedSize++;
      while (!(isEndLineEndFile(c))) {  // skim to end of line
        esp3d_log("Isn't Endline/File");
        c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
        ((ESP3DGcodeStream*)stream)->processedSize++;
      }
      while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
        esp3d_log("Is Whitespace");
        c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
        ((ESP3DGcodeStream*)stream)->processedSize++;
      }
    }
    esp3d_log("First char is: %c", c);
    stream->cursorPos = stream->processedSize;
    esp3d_log("Setting line start to: %d",
              (int)((ESP3DGcodeStream*)stream)->cursorPos);
    while (!(isEndLineEndFile(c) ||
             (((ESP3DGcodeStream*)stream)->processedSize >=
              ((ESP3DGcodeStream*)stream)
                  ->totalSize))) {  // while not end of line or end of file
      esp3d_log("Isn't Endline/File");
      if (c == ';') {  // reached a comment, skip to next line
        esp3d_log("Is comment");
        while (!(isEndLineEndFile(c))) {  // skim to end of line
          esp3d_log("Still comment");
          c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
          ((ESP3DGcodeStream*)stream)->processedSize++;
        }
      } else {  // no comment yet, read into command
        esp3d_log("Appending %c", c);
        _currentCommand[cmdPos] = c;
        cmdPos++;
        c = _readCharFromCommandBuffer((ESP3DGcodeStream*)stream);
        ((ESP3DGcodeStream*)stream)->processedSize++;
      }
    }
  }

  _currentCommand[cmdPos] = 0;
  esp3d_log("CurrCommand: %s", _currentCommand);
  if (strlen((char*)_currentCommand) == 0) {
    esp3d_log("Line Empty");
    return false;
  } else {
    esp3d_log("Command Read");
    return true;
  }

  return false;
}

/// @brief Calculate a checksum for the command provided. (Use crc.h instead?)
/// @param command Const Char Pointer to a buffer containing the null
/// terminated command.
/// @param commandSize The number of bytes of data in the command.
/// @return The checksum as an unsigned integer.
uint8_t ESP3DGCodeHostService::_Checksum(const char* command,
                                         uint32_t commandSize) {
  uint8_t checksum_val = 0;
  if (command == NULL) {
    return 0;
  }
  for (uint32_t i = 0; i < commandSize; i++) {
    checksum_val = checksum_val ^ ((uint8_t)command[i]);
  }
  return checksum_val;
}

// maybe rejig to use _currentCommand and cmdno (no vars necessary) //maybe
// absorb _Checksum() //use esp32 crc

/// @brief Apply a line number and checksum to the command referred to by the
/// pointer.
/// @param command Pointer to the command to be altered.
/// @param commandnb Line number of the command.
/// @return True if altered. False if command is empty.
bool ESP3DGCodeHostService::_CheckSumCommand(char* command,
                                             uint32_t commandnb) {
  if (command[0] == 0) return false;
  std::string stringy;

  char buffer[ESP_GCODE_HOST_COMMAND_LINE_BUFFER];
  strcpy(buffer, command);

  uint8_t crc = _Checksum(command, strlen(command));
  sprintf(command, "N%u %s *%u", (unsigned int)commandnb, buffer,
          (unsigned int)crc);
  return true;
}

/// @brief Go to the specified line start in the current print stream. For use
/// with resend requests.
/// @param line The line to move to.
/// @return True if line found and set correctly.
bool ESP3DGCodeHostService::_gotoLine(
    uint64_t line) {  // remember to account for M110 Nx commands
  // Not yet implemented
  return false;
}

bool ESP3DGCodeHostService::_parseResponse(
    ESP3DMessage* rx) {  //{ return false; }
  if (_isAck((char*)(rx->data))) {
    if (_awaitingAck) {
      _awaitingAck = false;
    } else {
      esp3d_log_w("Got ok but out of the query");
    }
  } else if (_isBusy((char*)(rx->data))) {
    esp3d_log("Has busy protocol, shorten timeout");
    _startTimeout = esp3d_hal::millis();
    _timeoutInterval = ESP_HOST_BUSY_TIMEOUT;
  } else if ((_resend_command_number = _resendCommandNumber((char*)rx->data)) !=
             0) {
    //_gotoLine(_current_stream.resendCommandNumber);
    if (_awaitingAck) {
      _awaitingAck = false;
    } else {
      esp3d_log_w("Got resend but out of the query");
    }
  } else if (_isError((char*)(rx->data))) {
    esp3d_log_e("Got Error: %s", ((char*)(rx->data)));
  }
  return true;
}

bool ESP3DGCodeHostService::_processRx(ESP3DMessage* rx) {
  if (_outputClient == ESP3DClientType::no_client) {
    esp3d_log_w("Output client not set, can't send: %s", (char*)(rx->data));
  } else if (rx->origin == _outputClient) {
    esp3d_log("Stream got the response: %s", (char*)(rx->data));
    _parseResponse(rx);
  } else {
    esp3d_log("Forwarding message from: %d", static_cast<uint8_t>(rx->origin));
    addStream((const char*)rx->data, rx->size, rx->authentication_level);
  }
  deleteMsg(rx);
  return true;
}

// needs reworking for however states end up
ESP3DGcodeHostState ESP3DGCodeHostService::getState() {
  ESP3DGcodeStream* stream = getCurrentStream();
  if (stream) {
    if (stream->state == ESP3DGcodeStreamState::paused)
      return ESP3DGcodeHostState::paused;

    return ESP3DGcodeHostState::processing;
  }
  return ESP3DGcodeHostState::idle;
}

// Get top active stream even it is paused
ESP3DGcodeStream* ESP3DGCodeHostService::getCurrentStream() {
  // get the first stream in the queue
  esp3d_log("There are currently %d scripts", _scripts.size());
  for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
    esp3d_log("Script type: %d", static_cast<uint8_t>((*script)->type));
    if ((*script)->type == ESP3DGcodeHostFileType::sd_stream ||
        (*script)->type == ESP3DGcodeHostFileType::fs_stream) {
      return (*script);
    }
  }
  esp3d_log("No script found");
  return nullptr;
}

ESP3DGcodeHostError ESP3DGCodeHostService::getErrorNum() {
  // Not yet implemented
  return ESP3DGcodeHostError::no_error;
}

ESP3DGCodeHostService::ESP3DGCodeHostService() {
  _started = false;
  _xHandle = NULL;
  _current_stream_ptr = NULL;
}
ESP3DGCodeHostService::~ESP3DGCodeHostService() { end(); }

void ESP3DGCodeHostService::process(ESP3DMessage* msg) {
  esp3d_log("Add message to queue");
  if (!addRxData(msg)) {
    // flush();
    if (!addRxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  } else {
    // flush();
  }
  //}
}

void ESP3DGCodeHostService::_updateOutputClient() {
  _outputClient = esp3dCommands.getOutputClient();
  esp3d_log("Output client is: %d", static_cast<uint8_t>(_outputClient));
}

bool ESP3DGCodeHostService::begin() {
  end();
  if (pthread_mutex_init(&_rx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for rx failed");
    return false;
  }
  setRxMutex(&_rx_mutex);

  if (pthread_mutex_init(&_tx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for tx failed");
    return false;
  }
  setTxMutex(&_tx_mutex);
  // Task is never stopped so no need to kill the task from outside
  _started = true;
  _updateOutputClient();
  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_gcode_host_task, "esp3d_gcode_host_task",
      ESP3D_GCODE_HOST_TASK_SIZE, NULL, ESP3D_GCODE_HOST_TASK_PRIORITY,
      &_xHandle, ESP3D_GCODE_HOST_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created GCode Host Task");
    esp3d_log("GCode Host service started");
    flush();
    return true;
  } else {
    esp3d_log_e("GCode Host Task creation failed");
    _started = false;
    return false;
  }
}

bool ESP3DGCodeHostService::_setStream() {
  if (!_scripts.empty()) {
    // esp3d_log("Processing %d script(s)", _scripts.size());

    if (_current_stream_ptr !=
        (ESP3DGcodeStream*)(_scripts.front())) {  // if we're not executing
                                                  // the front script
      esp3d_log("Switching streams");

      // if a different stream is loaded, close it, updating it's line
      // position if necessary
      if (_current_stream_ptr != nullptr) {
        if (_currentCommand[0] == 0) {
          (_current_stream_ptr->cursorPos) =
              (_current_stream_ptr->processedSize);
        } else {
          (_current_stream_ptr->processedSize) =
              (_current_stream_ptr->cursorPos);
          _currentCommand[0] = 0;
          if (_current_stream_ptr == &_current_stream) _command_number -= 1;
        }
        // if the open stream is a file, close the file
        if (isFileStream(
                _current_stream_ptr)) {  // need to handle command stream
                                         // changes too - re cursorPos
          esp3d_log("closing current file stream");  // REVIEW FILE HANDLING
          _closeFile((ESP3DGcodeStream*)_current_stream_ptr);
        }
      }

      _current_stream_ptr = (ESP3DGcodeStream*)(_scripts.front());
      if (isFileStream(_current_stream_ptr)) {
        _openFile((ESP3DGcodeStream*)_current_stream_ptr);
        _updateRingBuffer((ESP3DGcodeStream*)_current_stream_ptr);
      }
    }  // if we're streaming the same script, no need to do anything here

  } else if (_current_stream.dataStream !=
             "") {  // if no scripts, and we're currently streaming a
                    // print, execute print stream

    if (_current_stream_ptr != &_current_stream) {
      esp3d_log("Switching streams");
      if (_current_stream_ptr != nullptr) {
        if (isFileStream(_current_stream_ptr)) {
          esp3d_log("Closing current file stream at idx: %d",
                    (int)ftell(_File));
          _closeFile((ESP3DGcodeStream*)_current_stream_ptr);
        }
        if (_currentCommand[0] ==
            0) {  // if we just sent the loaded line, update cursorPos, else
                  // reset processed.
          (_current_stream_ptr->cursorPos) =
              (_current_stream_ptr->processedSize);
        } else {
          (_current_stream_ptr->processedSize) =
              (_current_stream_ptr->cursorPos);
          _currentCommand[0] = 0;
        }
      }

      _current_stream_ptr = &_current_stream;

      if (isFileStream(_current_stream_ptr)) {  // unnecessary if
        esp3d_log("Opening current print stream file and reading into buffer");
        for (int attempt = 0; attempt < 5; attempt++) {
          _openFile((ESP3DGcodeStream*)_current_stream_ptr);
          esp3d_log("fopen attempt %d", attempt + 1);
          if (_File != nullptr) break;
          vTaskDelay(pdTICKS_TO_MS(50));
        }

        if (_File != nullptr) {
          esp3d_log(
              "File Opened, addr: %s",
              ((ESP3DGcodeStream*)_current_stream_ptr)->dataStream.c_str());
        } else {
          esp3d_log_e("File failed to open");
        }

        if (_current_stream_ptr->cursorPos != 0) {  // move into open file?
          esp3d_log("Seeking to line start at: %d",
                    (int)(_current_stream_ptr->cursorPos));

          if (fseek(_File, (long)(_current_stream_ptr->cursorPos - 1),
                    SEEK_SET) != 0) {  // does this need -1?
            // do error stuff
            esp3d_log_e(
                "Failed to seek to correct line in file stream: %s",
                ((ESP3DGcodeStream*)_current_stream_ptr)->dataStream.c_str());
          } else {
            esp3d_log("Seek completed");
          }
        }

        _updateRingBuffer((ESP3DGcodeStream*)_current_stream_ptr);

        esp3d_log("File buffer read completed");
      }
    }

  } else {
    // esp3d_log("No Stream");
    _current_stream_ptr = nullptr;
  }

  return true;
}

void ESP3DGCodeHostService::handle() {
  if (_started) {
    // Check for notifications, set the current stream state
    if (ulTaskNotifyTake(pdTRUE, 0)) {
      if (ulTaskNotifyTakeIndexed(_xPauseNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received pause notification");
        if (_current_stream.dataStream != "") {
          _current_stream.state = ESP3DGcodeStreamState::pause;
        }
      }
      if (ulTaskNotifyTakeIndexed(_xResumeNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received resume notification");
        if (_current_stream.state == ESP3DGcodeStreamState::paused) {
          _current_stream.state = ESP3DGcodeStreamState::resume;
        } else if (_current_stream.state ==
                   ESP3DGcodeStreamState::pause) {  // if we haven't done pause
                                                    // actions yet, we don't
                                                    // need to do resume actions
                                                    // - just cancel the pause.
          _current_stream.state = ESP3DGcodeStreamState::read_line;
        } else {
          esp3d_log_w("No paused stream - nothing to resume");
        }
      }
      if (ulTaskNotifyTakeIndexed(_xAbortNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received abort notification");
        if (_current_stream.dataStream != "") {
          _current_stream.state = ESP3DGcodeStreamState::abort;
        }
      }
      if (ulTaskNotifyTakeIndexed(_xOutputChangeNotifyIndex, pdTRUE, 0)) {
        _updateOutputClient();
      }
    }

    _setStream();

    if (_current_stream_ptr != nullptr) {
      switch (_current_stream_ptr->state) {
        case ESP3DGcodeStreamState::end:
          esp3d_log("Ending Stream");
          _endStream(_current_stream_ptr);
          _current_stream_ptr = nullptr;
          break;

        case ESP3DGcodeStreamState::start:
          esp3d_log("Starting Stream");
          _startStream(_current_stream_ptr);  // may be worth eliminating -
                                              // only checks total file size
          _current_stream_ptr->state =
              ESP3DGcodeStreamState::read_line;  // Put this in
                                                 // _startStream()?
          __attribute__((fallthrough));

        case ESP3DGcodeStreamState::read_line:
          if (_currentCommand[0] == 0) {
            esp3d_log("Reading Line");
            if (_readNextCommand(_current_stream_ptr)) {
              if (esp3dCommands.is_esp_command(
                      (uint8_t*)(&(_currentCommand[0])),
                      strlen((char*)(&(_currentCommand[0]))))) {
                esp3d_log("Is ESP Command");
                _next_state = _current_state;  // if waiting_for_ack we can send
                                               // this and check on ack after we
                                               // send any esp commands
                _current_state = ESP3DGcodeHostState::send_esp_command;
                strcat((char*)_currentCommand, "\n");
              } else {
                esp3d_log("Is Gcode Command");
                // add line no and checksum if appropriate
                if (_current_stream_ptr == &_current_stream) {
                  _command_number += 1;
                  _CheckSumCommand((char*)_currentCommand, _command_number);
                }
                strcat((char*)_currentCommand, "\n");
                if (_current_state == ESP3DGcodeHostState::wait_for_ack) {
                  _next_state = ESP3DGcodeHostState::send_gcode_command;
                } else {
                  _current_state = ESP3DGcodeHostState::send_gcode_command;
                }
              }
            } else {  // returns false at end of file/script
              esp3d_log("Stream Empty");
              _next_state = ESP3DGcodeHostState::idle;
              _current_stream_ptr->state = ESP3DGcodeStreamState::end;
            }
          }
          break;

        case ESP3DGcodeStreamState::pause:
          // add pause script to _scripts
          //_current_stream_ptr -> state = ESP3DGcodeStreamState::paused;
          _current_stream.state = ESP3DGcodeStreamState::paused;
          _streamData(_pause_script.c_str(), _current_stream_ptr->auth_type,
                      true);
          break;

        case ESP3DGcodeStreamState::resume:
          // add resume script to _scripts
          //_current_stream_ptr -> state = ESP3DGcodeStreamState::read_line;
          _current_stream.state = ESP3DGcodeStreamState::read_line;
          _streamData(_resume_script.c_str(), _current_stream_ptr->auth_type,
                      true);
          break;

        case ESP3DGcodeStreamState::abort:
          // add abort script to _scripts and end stream
          //_current_stream_ptr -> state = ESP3DGcodeStreamState::end; // Does
          // this
          // need to apply only to currentPrintStream?
          _current_stream.state = ESP3DGcodeStreamState::end;
          _streamData(_stop_script.c_str(), _current_stream_ptr->auth_type,
                      true);
          break;

        case ESP3DGcodeStreamState::paused:  // might be worth updating the
                                             // ring buffer here
          _current_state =
              ESP3DGcodeHostState::idle;  // may need renaming to better
                                          // reflect current state of stream
          break;

        default:
          break;
      }
    }

    // esp3d_log("Host state: %d", static_cast<uint8_t>(state));
    switch (_current_state) {
      case ESP3DGcodeHostState::idle:
        // esp3d_log("Idling");
        // do nothing, either no stream, or stream is paused
        // _current_state = _next_state; //shouldn't be necessary, _next_state
        // should only be used with wait_for_ack esp3d_log("Wait");
        // vTaskDelay?
        break;

      case ESP3DGcodeHostState::wait_for_ack:
        if (!_awaitingAck) {
          _current_state = _next_state;
          _next_state = ESP3DGcodeHostState::idle;
        } else {
          if (_current_stream_ptr != nullptr) {
            if (isFileStream(_current_stream_ptr)) {
              _updateRingBuffer((ESP3DGcodeStream*)_current_stream_ptr);
            }
          }
          if (esp3d_hal::millis() - _startTimeout > _timeoutInterval) {
            _current_state = ESP3DGcodeHostState::error;  // unhandled
            esp3d_log_e("Timeout waiting for ok");
          }
        }
        break;

      case ESP3DGcodeHostState::send_gcode_command: {
        if (!_awaitingAck) {  // if else shouldn't really be necessary here,
                              // as it should end up in the wait for ack
                              // state. just a precaution for now, remove
                              // later.
          esp3d_log("Sending to output client: %s", &_currentCommand[0]);
          if (_outputClient == ESP3DClientType::serial) {
            uart_write_bytes(ESP3D_SERIAL_PORT, &_currentCommand,
                             strlen((char*)&(_currentCommand[0])));
#if ESP3D_USB_SERIAL_FEATURE
          } else if (_outputClient == ESP3DClientType::usb_serial) {
            ESP3DMessage* msg = newMsg(ESP3DClientType::stream, _outputClient,
                                       (uint8_t*)(&(_currentCommand[0])),
                                       strlen((char*)&(_currentCommand[0])),
                                       _current_stream_ptr->auth_type);
            usbSerialClient.process(msg);  // may just dispatch to output client
#endif
          }

          _startTimeout = esp3d_hal::millis();
          _awaitingAck = true;
          _currentCommand[0] = 0;
          _current_state = ESP3DGcodeHostState::wait_for_ack;
          _next_state = ESP3DGcodeHostState::idle;
        } else {
          _current_state = ESP3DGcodeHostState::wait_for_ack;
        }
        break;
      }

      case ESP3DGcodeHostState::send_esp_command: {
        ESP3DMessage* msg =
            newMsg(ESP3DClientType::stream, ESP3DClientType::command,
                   (uint8_t*)(&(_currentCommand[0])),
                   strlen((char*)&(_currentCommand[0])),
                   _current_stream_ptr->auth_type);
        esp3dCommands.process(msg);
        _currentCommand[0] = 0;
        _current_state = _next_state;
        _next_state = ESP3DGcodeHostState::idle;
        break;
      }

      default:
        // error maybe?
        break;
    }

    // should probably put this in a function

    while (getRxMsgsCount() > 0) {
      ESP3DMessage* msg = popRx();
      esp3d_log("RX popped");
      if (msg) {
        _processRx(msg);
      }
    }
  }
}

void ESP3DGCodeHostService::flush() {  // should only be called when no handle
                                       // task is running
  uint8_t loopCount = 10;
  while (loopCount && ((getRxMsgsCount() > 0) || (getTxMsgsCount() > 0))) {
    esp3d_log("flushing Tx messages");
    loopCount--;
    handle();
  }
}

void ESP3DGCodeHostService::end() {
  if (_started) {
    flush();
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_log("Clearing queue Tx messages");
    clearTxQueue();
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (pthread_mutex_destroy(&_tx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for tx failed");
    }
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for rx failed");
    }
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }

  _current_stream = ESP3DGcodeStream{};
  _current_stream_ptr = nullptr;
  _scripts.clear();
}