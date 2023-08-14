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
// #define ESP3D_GCODE_HOST_FEATURE
#ifdef ESP3D_GCODE_HOST_FEATURE
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
    const char* file) {  // maybe this can also check for invalid file
                         // extensions if not done elsewhere
  if (file[0] == '/') {
    if (strstr(file, "/sd/") == file) {
#if ESP3D_SD_CARD_FEATURE
      return ESP3DGcodeHostFileType::sd_card;
#else
      return ESP3DGcodeHostFileType::invalid;  // maybe should return error?
#endif  // ESP3D_SD_CARD_FEATURE
    } else {
      return ESP3DGcodeHostFileType::filesystem;
    }
  } else {
    if (strstr(file, "\n") != nullptr) {  // can we use \n instead of ;?
      return ESP3DGcodeHostFileType::multiple_commands;
    } else {
      if (strlen(file) != 0)
        return ESP3DGcodeHostFileType::single_command;
      else
        return ESP3DGcodeHostFileType::invalid;
    }
  }
}

void ESP3DGCodeHostService::updateScripts() {
  char buffer[SIZE_OF_SCRIPT + 1];
  _stop_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_stop_script, buffer, SIZE_OF_SCRIPT);
  _resume_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_resume_script, buffer, SIZE_OF_SCRIPT);
  _pause_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_pause_script, buffer, SIZE_OF_SCRIPT);
}

bool ESP3DGCodeHostService::newStream(
    const char* command, size_t length,
    ESP3DAuthenticationLevel authentication_level, bool isPrintStream) {
  // ESP700 will have isPrintStream == true, terminal commands and macros use
  // default of false.

  uint8_t* data = (uint8_t*)pvPortMalloc(sizeof(uint8_t) * (length + 5));
  if (data == nullptr) {
    esp3d_log_e("Failed to allocate memory for stream data.");
    return false;
  }

  ESP3DMessage* msg =
      newMsg(ESP3DClientType::stream,
             ESP3DClientType::stream,  // might be a bad way to do it, let me
                                       // know what you think.
             authentication_level);
  if (msg == nullptr) {
    esp3d_log_e("Failed to create stream request message.");
    return false;
  }

  if (isPrintStream == true) {
    // Check here to see if print is in progres - reject if so. Awaiting updated
    // getCurrentStream/getState functions.
    if (false) {
      esp3d_log_w("Print stream already in progress");
      return false;
    }

    strncpy((char*)data,
            "PNT:", 5);  // prefixes allow for file macros if desired. ex:
                         // "PNT:/sd/benchy.gcode" vs "CMD:/sd/macrostuff.gcode"
  } else {  // cmd file doesn't get rejected when print in progress, pnt does
    strncpy((char*)data, "CMD:", 5);
  }
  memcpy(&data[4], command, length);
  data[length + 4] = 0;

  msg->data = data;
  msg->size = strlen((char*)data) + 1;
  process(msg);
  return true;
}

bool ESP3DGCodeHostService::_streamFile(const char* file,
                                        ESP3DAuthenticationLevel auth_type,
                                        bool executeAsMacro,
                                        bool executeFirst) {
  esp3d_log("Processing stream request: %s, with authentication level=%d", file,
            static_cast<uint8_t>(
                auth_type));  // Need to implement authorisation check here
  ESP3DGcodeHostFileType type = _getStreamType(file);
  esp3d_log("File type: %d", static_cast<uint8_t>(type));

  if (executeAsMacro) {
    if (type == ESP3DGcodeHostFileType::filesystem) {
      type = ESP3DGcodeHostFileType::script;
#if ESP3D_SD_CARD_FEATURE
    } else if (type == ESP3DGcodeHostFileType::sd_card) {
      type = ESP3DGcodeHostFileType::sd_script;
#endif  // ESP3D_SD_CARD_FEATURE
    }
  }

  switch (type) {
    case ESP3DGcodeHostFileType::filesystem:
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostFileType::sd_card:
#endif  // ESP3D_SD_CARD_FEATURE
    {
      if (getCurrentStream() != nullptr) return false;
      for (int attempt = 0; attempt < 5; attempt++) {
        _currentPrintStream._fileName = (uint8_t*)pvPortMalloc(
            sizeof(char) *
            (strlen(file) + 1));  // Need to limit memory usage -- Does task
                                  // memory allocation do this?
        if (_currentPrintStream._fileName != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log_w("Memory allocation attempt %d failed", attempt + 1);
      }
      if (_currentPrintStream._fileName == nullptr) {
        esp3d_log_e("Memory allocation failed");
        vPortFree(*(executeFirst ? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase(
            (executeFirst ? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else {
        esp3d_log("Memory allocated at address: %d",
                  (unsigned int)(_currentPrintStream._fileName));
      }
      strncpy((char*)_currentPrintStream._fileName, file, strlen(file) + 1);
      _currentPrintStream.type = type;
      _currentPrintStream.id = esp3d_hal::millis();
      _currentPrintStream.auth_type = auth_type;
      _currentPrintStream.commandNumber = 0;
      _currentPrintStream.resendCommandNumber = 0;
      _currentPrintStream.state = ESP3DGcodeStreamState::start;
      esp3d_log("Stream type: %d",
                static_cast<uint8_t>(_currentPrintStream.type));

      // send line number reset command if commencing a print stream.
      const char lineno_reset[] =
          "M110 N0\n";  // Is there a better way to store this const?
      _streamFile(lineno_reset, auth_type);

      return true;
    } break;

    case ESP3DGcodeHostFileType::script:
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostFileType::sd_script:
#endif  // ESP3D_SD_CARD_FEATURE
    {
      if (executeFirst) {
        _scripts.push_front(new ESP3DGcodeFileStream());
      } else {
        _scripts.push_back(new ESP3DGcodeFileStream());
      }
      ESP3DGcodeFileStream* streamPointer =
          (executeFirst ? ((ESP3DGcodeFileStream*)(_scripts.front()))
                        : ((ESP3DGcodeFileStream*)(_scripts.back())));
      for (int attempt = 0; attempt < 5; attempt++) {
        streamPointer->_fileName =
            (uint8_t*)pvPortMalloc(sizeof(char) * (strlen(file) + 1));
        if (streamPointer->_fileName != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log_w("Memory allocation attempt %d failed", attempt + 1);
      }
      if (streamPointer->_fileName == nullptr) {
        esp3d_log_e("Memory allocation failed");
        vPortFree(*(executeFirst ? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase(
            (executeFirst ? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else {
        esp3d_log("Memory allocated at address: %d",
                  (unsigned int)(streamPointer->_fileName));
      }
      strncpy((char*)(streamPointer->_fileName), file, strlen(file) + 1);
      streamPointer->type = type;
      streamPointer->id = esp3d_hal::millis();
      streamPointer->auth_type = auth_type;
      streamPointer->commandNumber = 0;
      streamPointer->resendCommandNumber = 0;
      streamPointer->state = ESP3DGcodeStreamState::start;
      esp3d_log("Script type: %d", static_cast<uint8_t>(streamPointer->type));
      return true;
    } break;

    case ESP3DGcodeHostFileType::single_command:
    case ESP3DGcodeHostFileType::multiple_commands: {
      if (executeFirst) {
        _scripts.push_front(new ESP3DGcodeCommandStream());
      } else {
        _scripts.push_back(new ESP3DGcodeCommandStream());
      }
      ESP3DGcodeCommandStream* streamPointer =
          (executeFirst ? ((ESP3DGcodeCommandStream*)(_scripts.front()))
                        : ((ESP3DGcodeCommandStream*)(_scripts.back())));
      for (int attempt = 0; attempt < 5; attempt++) {
        streamPointer->commandBuffer =
            (uint8_t*)pvPortMalloc(sizeof(char) * (strlen(file) + 1));
        if (streamPointer->commandBuffer != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log("malloc attempt %d failed", attempt + 1);
      }
      if (streamPointer->commandBuffer == nullptr) {
        esp3d_log("malloc failed");
        vPortFree(*(executeFirst ? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase(
            (executeFirst ? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else {
        esp3d_log("malloc'd");
        esp3d_log("Buffer address: %d",
                  (unsigned int)(streamPointer->commandBuffer));
      }
      esp3d_log("Command length: %d", (unsigned int)strlen(file));
      strncpy((char*)streamPointer->commandBuffer, file, strlen(file) + 1);
      esp3d_log("copied");
      streamPointer->type = type;
      streamPointer->id = esp3d_hal::millis();
      streamPointer->auth_type = auth_type;
      streamPointer->state = ESP3DGcodeStreamState::start;
      esp3d_log("id: %d", (int)streamPointer->id);
      esp3d_log("File type: %d", static_cast<uint8_t>(streamPointer->type));
      return true;
    } break;

    case ESP3DGcodeHostFileType::invalid:
      return false;
      break;

    default:
      return false;
      break;
  }
  return false;
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

// ##################### Parsing Functions ########################

bool ESP3DGCodeHostService::_isAck(const char* cmd) {
  if ((strstr(cmd, "ok") != nullptr)) {
    esp3d_log("Got ok");
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isBusy(const char* cmd) {
  if ((strstr(cmd, "busy:") != nullptr)) {
    esp3d_log("Got busy");
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isError(const char* cmd) {
  return false;  // need info on printer error formats
}

uint64_t ESP3DGCodeHostService::_resendCommandNumber(
    const char* cmd) {  // we should rename this
  char* p = nullptr;
  if (((p = strstr(cmd, "resend:")) != nullptr)) {
    esp3d_log("Got resend");
    return atoi(p + 7);
  }
  return 0;
}

bool ESP3DGCodeHostService::_isEmergencyParse(const char* cmd) {
  if ((strstr(cmd, "M112") != nullptr) || (strstr(cmd, "M108") != nullptr)) {
    return true;
  }
  return false;
}

// ##################### File/Script Handling Functions ########################

/// @brief Opens the file assosciated with the provided file stream, and seeks
/// to the correct position.
/// @param stream Pointer to the file stream to be opened.
/// @return True if file opened successfully.
bool ESP3DGCodeHostService::_openFile(ESP3DGcodeFileStream* stream) {
  esp3d_log("File name is %s", stream->_fileName);
  if (globalFs.accessFS((char*)(stream->_fileName))) {
    if (globalFs.exists((char*)(stream->_fileName))) {
      esp3d_log("File exists");
      stream->streamFile = globalFs.open((char*)stream->_fileName, "r");
      if ((stream->streamFile) != nullptr) {
        if (_current_stream->lineStart != 0) {
          if (fseek(((ESP3DGcodeFileStream*)_current_stream)->streamFile,
                    (long)_current_stream->lineStart, SEEK_SET) !=
              0) {  // this would need adjusting if concurrrent file access is
                    // implemented (seek to buffer end instead)
            esp3d_log_e("Failed to seek to correct position in file: %s",
                        ((ESP3DGcodeFileStream*)_current_stream)->_fileName);
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
bool ESP3DGCodeHostService::_closeFile(ESP3DGcodeFileStream* stream) {
  if (stream->streamFile == nullptr) {
    esp3d_log_e("No file to close");
    return false;
  }

  esp3d_log("Closing File: %s", stream->_fileName);
  globalFs.close((stream->streamFile), (char*)(stream->_fileName));
  globalFs.releaseFS((char*)(stream->_fileName));

  return true;
}

/// @brief Update the ring buffer with data from the provided file stream.
/// @param stream Pointer to the file stream to update the buffer with.
/// @return True if buffer successfully updated.
bool ESP3DGCodeHostService::_updateRingBuffer(ESP3DGcodeFileStream* stream) {
  // _bufferSeam: This is the index of the earliest byte (end) in the buffer.

  // if the correct stream is not buffered, reset and fill the buffer
  if (_bufferedStream != stream) {
    // Set indexes and load new data
    _bufferPos = 0;
    _bufferedStream = stream;
    _bufferFresh = true;

    if ((_bufferSeam = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH,
                             (stream->streamFile))) == RING_BUFFER_LENGTH) {
      _bufferSeam = 0;
    } else {
      _ringBuffer[_bufferSeam] = 0;
    }
    esp3d_log("Buffered stream switched");

  } else {
    if ((_bufferPos == _bufferSeam) && (_bufferFresh == true)) {
      // esp3d_log("Buffer is full");
      return false;  // return, buffer is already full of fresh data
    }
    if ((_ringBuffer[_bufferSeam] == 0) || (_ringBuffer[_bufferPos] == 0)) {
      esp3d_log("File finished");
      return false;  // return, file finished
    }

    esp3d_log("Updating buffer from: %s", _ringBuffer);

    int empty = 0;
    int read = 0;
    // if we've reached the seam we need to read a whole buffer
    if (_bufferPos == _bufferSeam) {
      empty = RING_BUFFER_LENGTH;  // maybe not needed
      read = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH,
                   (stream->streamFile));
      _bufferPos = 0;
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

      // if we're after the seam do a single read to update from the seam to our
      // position
    } else if (_bufferPos > _bufferSeam) {
      empty = _bufferPos - _bufferSeam;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char), empty,
                   (stream->streamFile));
      if (read == empty) {
        _bufferSeam = _bufferPos;
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
      empty = RING_BUFFER_LENGTH - _bufferSeam + _bufferPos;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char),
                   RING_BUFFER_LENGTH - _bufferSeam, (stream->streamFile));
      if (_bufferPos > 0) {
        read +=
            fread(_ringBuffer, sizeof(char), _bufferPos, (stream->streamFile));
      }
      if (read == empty) {
        _bufferSeam = _bufferPos;
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

/// @brief Read a single character from the ring buffer, from the provided file
/// stream. Will update contents of the buffer as required.
/// @param stream Pointer to the file stream to read a character from.
/// @return Current character from the buffer. Will return 0 at the end of the
/// stream.
char ESP3DGCodeHostService::_readCharFromRingBuffer(
    ESP3DGcodeFileStream* stream) {
  // if we're at the seam, and not fresh, whole buffer is used and needs
  // updating.
  if (_bufferFresh == true) {
    if (_bufferPos != _bufferSeam) {
      _bufferFresh = false;
    }
  } else if (_bufferPos == _bufferSeam) {
    if (_bufferFresh == false) _updateRingBuffer(stream);
  }

  char c = _ringBuffer[_bufferPos];
  (_bufferPos == RING_BUFFER_LENGTH - 1) ? _bufferPos = 0 : _bufferPos++;
  return c;  // if this returns NULL, we're at endfile
}

/// @brief Read a single character from the buffer of the provided command
/// stream.
/// @param stream Pointer to the command buffer to read a character from.
/// @return Current character from the buffer. Will return 0 at the end of the
/// stream.
char ESP3DGCodeHostService::_readCharFromCommandBuffer(
    ESP3DGcodeCommandStream* stream) {
  char c = stream->commandBuffer[((ESP3DGcodeCommandStream*)stream)->bufferPos];
  stream->bufferPos++;
  return c;
}

// ##################### Stream Handling Functions ########################

// would be better to eliminate this function, and find the size of the stream
// elsewhere. (stream() Maybe)
bool ESP3DGCodeHostService::_startStream(ESP3DGcodeStream* stream) {
  // function may not be needed after stream() <------ do this for sure.
  // nothing to be done if command stream
  stream->lineStart = 0;
  if (isFileStream(stream)) {
    if (((ESP3DGcodeFileStream*)stream)->_fileName != nullptr) {
      int pos = ftell(((ESP3DGcodeFileStream*)stream)->streamFile) +
                1;  // we can get rid of this if we make sure to update the
                    // buffer after this function is called.
      fseek(((ESP3DGcodeFileStream*)stream)->streamFile, 0, SEEK_END);
      ((ESP3DGcodeCommandStream*)stream)->totalSize =
          ftell(((ESP3DGcodeFileStream*)stream)->streamFile) +
          1;  // +1? for size instead of end index
      fseek(((ESP3DGcodeFileStream*)stream)->streamFile, pos, SEEK_SET);
      esp3d_log("Size is: %d",
                (unsigned int)((ESP3DGcodeFileStream*)stream)->totalSize);
      return true;
    }
    esp3d_log_e("Stream file not currently open");
    return false;
  } else if (isCommandStream(stream)) {
    ((ESP3DGcodeCommandStream*)stream)->totalSize =
        strlen((char*)((ESP3DGcodeCommandStream*)stream)->commandBuffer);
    esp3d_log("Size is: %d",
              (unsigned int)((ESP3DGcodeCommandStream*)stream)->totalSize);
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
  esp3d_log("Freeing stream buffers");
  if (isFileStream(stream)) {
    _closeFile((
        ESP3DGcodeFileStream*)stream);  // Behaviour may need altering depending
                                        // on consecutive storage access
    vPortFree(((ESP3DGcodeFileStream*)stream)->_fileName);
    // memset(_ringBuffer, 0 , RING_BUFFER_LENGTH); //shouldn't be necessary
    if (stream == &_currentPrintStream) {
      *stream = (ESP3DGcodeFileStream){};  // Take a look over this, I don't
                                           // think we need all of these lines
      _currentPrintStream._fileName = nullptr;
      _bufferedStream = nullptr;
    } else {
      esp3d_log("Erasing script from queue");
      vPortFree(*(_scripts.begin()));
      _scripts.erase(_scripts.begin());
    }
  } else if (isCommandStream(stream)) {
    vPortFree((((ESP3DGcodeCommandStream*)stream)->commandBuffer));
    ((ESP3DGcodeCommandStream*)stream)->commandBuffer = nullptr;
    esp3d_log("Erasing script from queue");
    vPortFree(*(_scripts.begin()));
    _scripts.erase(_scripts.begin());
  }
  esp3d_log("Freed");

  return true;
}

/// @brief Read the next command line from the provided stream into
/// _currentCommand. Skips past comments and empty lines.
/// @param stream Pointer to the command or file stream to be read from.
/// @return True if command is read, False if no command read (end of stream).
bool ESP3DGCodeHostService::_readNextCommand(ESP3DGcodeStream* stream) {
  stream->lineStart = stream->processedSize;
  uint32_t cmdPos = 0;

  if (isFileStream(stream)) {
    char c = (char)0;

    do {
      c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
      ((ESP3DGcodeFileStream*)stream)->processedSize++;

      while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
        c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
        ((ESP3DGcodeFileStream*)stream)->processedSize++;
      }
      while (c == ';') {  // while its a full line comment, skim to next line
        while (!(isEndLineEndFile(c))) {
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
          ((ESP3DGcodeFileStream*)stream)->processedSize++;
        }
        while (isWhiteSpace(c)) {  // ignore leading spaces and empty lines
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
          ((ESP3DGcodeFileStream*)stream)->processedSize++;
        }
      }
      stream->lineStart = stream->processedSize;

      while (!(isEndLineEndFile(c) ||
               (((ESP3DGcodeFileStream*)stream)->processedSize >=
                ((ESP3DGcodeFileStream*)stream)
                    ->totalSize))) {  // while not end of line or end of file
        if (c == ';') {               // reached a comment, skim to next line
          while (!(isEndLineEndFile(c))) {
            c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
            ((ESP3DGcodeFileStream*)stream)->processedSize++;
          }
        } else {  // no comment yet, read into command
          _currentCommand[cmdPos] = c;
          cmdPos++;
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)stream);
          ((ESP3DGcodeFileStream*)stream)->processedSize++;
        }
      }

    } while (!isEndLineEndFile(c));

  } else if (isCommandStream(stream)) {
    esp3d_log("Is command stream");
    esp3d_log("Buffer size is: %d",
              strlen((char*)((ESP3DGcodeCommandStream*)stream)->commandBuffer));
    char c = (char)0;
    if (((ESP3DGcodeCommandStream*)stream)->bufferPos >=
        stream->totalSize) {  // if we reach the end of the buffer, the command
                              // stream is done
      esp3d_log("End of Buffer");
      return false;
    }

    c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
    ((ESP3DGcodeCommandStream*)stream)->processedSize++;
    while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
      esp3d_log("Is Whitespace");
      c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
      ((ESP3DGcodeCommandStream*)stream)->processedSize++;
    }
    while (c ==
           ';') {  // while its a full line comment, read on to the next line
      c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
      ((ESP3DGcodeCommandStream*)stream)->processedSize++;
      while (!(isEndLineEndFile(c))) {  // skim to end of line
        esp3d_log("Isn't Endline/File");
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
        ((ESP3DGcodeCommandStream*)stream)->processedSize++;
      }
      while (isWhiteSpace(c)) {  // ignore any leading spaces and empty lines
        esp3d_log("Is Whitespace");
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
        ((ESP3DGcodeCommandStream*)stream)->processedSize++;
      }
    }
    esp3d_log("First char is: %c", c);
    stream->lineStart = stream->processedSize;
    esp3d_log("Setting line start to: %d",
              (int)((ESP3DGcodeFileStream*)stream)->lineStart);
    while (!(isEndLineEndFile(c) ||
             (((ESP3DGcodeCommandStream*)stream)->processedSize >=
              ((ESP3DGcodeCommandStream*)stream)
                  ->totalSize))) {  // while not end of line or end of file
      esp3d_log("Isn't Endline/File");
      if (c == ';') {  // reached a comment, skip to next line
        esp3d_log("Is comment");
        while (!(isEndLineEndFile(c))) {  // skim to end of line
          esp3d_log("Still comment");
          c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
          ((ESP3DGcodeCommandStream*)stream)->processedSize++;
        }
      } else {  // no comment yet, read into command
        esp3d_log("Appending %c", c);
        _currentCommand[cmdPos] = c;
        cmdPos++;
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*)stream);
        ((ESP3DGcodeCommandStream*)stream)->processedSize++;
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
/// @param command Const Char Pointer to a buffer containing the null terminated
/// command.
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
  } else if ((_currentPrintStream.resendCommandNumber =
                  _resendCommandNumber((char*)rx->data)) != 0) {
    //_gotoLine(_currentPrintStream.resendCommandNumber);
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
  if ((rx->origin == ESP3DClientType::serial) ||
      (rx->origin ==
       ESP3DClientType::usb_serial)) {  // this will be performed for every
                                        // command sent whilst printing,
                                        // getOutputClient() is unnecessarily
                                        // intensive
    esp3d_log("Stream got the response: %s", (char*)(rx->data));
    _parseResponse(rx);
  } else if (rx->origin == ESP3DClientType::stream) {
    _streamFile((char*)&(rx->data[4]), rx->authentication_level);
  } else {
    esp3d_log("Forwarding message from: %d", static_cast<uint8_t>(rx->origin));
    newStream((const char*)rx->data, rx->size,
              rx->authentication_level);  // placeholder until external code
                                          // uses newStream()
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

ESP3DGcodeStream*
ESP3DGCodeHostService::getCurrentStream(  /// CHECK OVER THIS FUNCTION -- Not
                                          /// very thread safe, what is best way
                                          /// to send string? Msg?
    ESP3DGcodeHostFileType type) {
  // Do we care about reporting the number of terminal commands in the queue?

  if (type == ESP3DGcodeHostFileType::active) {
    if (_currentPrintStream._fileName == nullptr) {
      return nullptr;
    } else {
      return &_currentPrintStream;
    }
  }

  esp3d_log("There are currently %d scripts", _scripts.size());
  for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
    esp3d_log("Script type: %d", static_cast<uint8_t>((*script)->type));

    if ((type == ESP3DGcodeHostFileType::filesystem &&
         ((*script)->type == ESP3DGcodeHostFileType::sd_card ||
          (*script)->type == ESP3DGcodeHostFileType::filesystem)) ||
        (type == (*script)->type)) {
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
  _current_stream = NULL;
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

    if (_current_stream !=
        (ESP3DGcodeStream*)(_scripts.front())) {  // if we're not executing the
                                                  // front script
      esp3d_log("Switching streams");

      // if a different stream is loaded, close it, updating it's line position
      // if necessary
      if (_current_stream != nullptr) {
        if (_currentCommand[0] == 0) {
          (_current_stream->lineStart) = (_current_stream->processedSize);
        } else {
          (_current_stream->processedSize) = (_current_stream->lineStart);
          _currentCommand[0] = 0;
          if (_current_stream == &_currentPrintStream)
            _currentPrintStream.commandNumber -= 1;
        }
        // if the open stream is a file, close the file
        if (isFileStream(_current_stream)) {  // need to handle command stream
                                              // changes too - re lineStart
          esp3d_log("closing current file stream");  // REVIEW FILE HANDLING
          _closeFile((ESP3DGcodeFileStream*)_current_stream);
        }
      }

      _current_stream = (ESP3DGcodeStream*)(_scripts.front());
      if (isFileStream(_current_stream)) {
        _openFile((ESP3DGcodeFileStream*)_current_stream);
        _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);
      }
    }  // if we're streaming the same script, no need to do anything here

  } else if (_currentPrintStream._fileName !=
             nullptr) {  // if no scripts, and we're currently streaming a
                         // print, execute print stream

    if (_current_stream != &_currentPrintStream) {
      esp3d_log("Switching streams");
      if (_current_stream != nullptr) {
        if (isFileStream(_current_stream)) {
          esp3d_log(
              "Closing current file stream at idx: %d",
              (int)ftell(((ESP3DGcodeFileStream*)_current_stream)->streamFile));
          _closeFile((ESP3DGcodeFileStream*)_current_stream);
        }
        if (_currentCommand[0] ==
            0) {  // if we just sent the loaded line, update linestart, else
                  // reset processed.
          (_current_stream->lineStart) = (_current_stream->processedSize);
        } else {
          (_current_stream->processedSize) = (_current_stream->lineStart);
          _currentCommand[0] = 0;
        }
      }

      _current_stream = &_currentPrintStream;

      if (isFileStream(_current_stream)) {  // unnecessary if
        esp3d_log("Opening current print stream file and reading into buffer");
        for (int attempt = 0; attempt < 5; attempt++) {
          _openFile((ESP3DGcodeFileStream*)_current_stream);
          esp3d_log("fopen attempt %d", attempt + 1);
          if (((ESP3DGcodeFileStream*)_current_stream)->streamFile != nullptr)
            break;
          vTaskDelay(pdTICKS_TO_MS(50));
        }

        if (((ESP3DGcodeFileStream*)_current_stream)->streamFile != nullptr) {
          esp3d_log("File Opened, addr: %d",
                    (int)((ESP3DGcodeFileStream*)_current_stream)->_fileName);
        } else {
          esp3d_log_e("File failed to open");
        }

        if (_current_stream->lineStart != 0) {  // move into open file?
          esp3d_log("Seeking to line start at: %d",
                    (int)(_current_stream->lineStart));

          if (fseek(((ESP3DGcodeFileStream*)_current_stream)->streamFile,
                    (long)(_current_stream->lineStart - 1),
                    SEEK_SET) != 0) {  // does this need -1?
            // do error stuff
            esp3d_log_e("Failed to seek to correct line in file stream: %s",
                        ((ESP3DGcodeFileStream*)_current_stream)->_fileName);
          } else {
            esp3d_log("Seek completed");
          }
        }

        _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);

        esp3d_log("File buffer read completed");
      }
    }

  } else {
    // esp3d_log("No Stream");
    _current_stream = nullptr;
  }

  return true;
}

void ESP3DGCodeHostService::handle() {
  if (_started) {
    // Check for notifications, set the current stream state
    if (ulTaskNotifyTake(pdTRUE, 0)) {
      if (ulTaskNotifyTakeIndexed(_xPauseNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received pause notification");
        if (_currentPrintStream._fileName != nullptr) {
          _currentPrintStream.state = ESP3DGcodeStreamState::pause;
        }
      }
      if (ulTaskNotifyTakeIndexed(_xResumeNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received resume notification");
        if (_currentPrintStream.state == ESP3DGcodeStreamState::paused) {
          _currentPrintStream.state = ESP3DGcodeStreamState::resume;
        } else if (_currentPrintStream.state ==
                   ESP3DGcodeStreamState::pause) {  // if we haven't done pause
                                                    // actions yet, we don't
                                                    // need to do resume actions
                                                    // - just cancel the pause.
          _currentPrintStream.state = ESP3DGcodeStreamState::read_line;
        } else {
          esp3d_log_w("No paused stream - nothing to resume");
        }
      }
      if (ulTaskNotifyTakeIndexed(_xAbortNotifyIndex, pdTRUE, 0)) {
        esp3d_log("Received abort notification");
        if (_currentPrintStream._fileName != nullptr) {
          _currentPrintStream.state = ESP3DGcodeStreamState::abort;
        }
      }
    }

    _setStream();

    if (_current_stream != nullptr) {
      switch (_current_stream->state) {
        case ESP3DGcodeStreamState::end:
          esp3d_log("Ending Stream");
          _endStream(_current_stream);
          _current_stream = nullptr;
          break;

        case ESP3DGcodeStreamState::start:
          esp3d_log("Starting Stream");
          _startStream(_current_stream);  // may be worth eliminating - only
                                          // checks total file size
          _current_stream->state =
              ESP3DGcodeStreamState::read_line;  // Put this in _startStream()?
          __attribute__((fallthrough));

        case ESP3DGcodeStreamState::read_line:
          if (_currentCommand[0] == 0) {
            esp3d_log("Reading Line");
            if (_readNextCommand(_current_stream)) {
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
                if (_current_stream == &_currentPrintStream) {
                  _currentPrintStream.commandNumber += 1;
                  _CheckSumCommand((char*)_currentCommand,
                                   _currentPrintStream.commandNumber);
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
              _current_stream->state = ESP3DGcodeStreamState::end;
            }
          }
          break;

        case ESP3DGcodeStreamState::pause:
          // add pause script to _scripts
          //_current_stream -> state = ESP3DGcodeStreamState::paused;
          _currentPrintStream.state = ESP3DGcodeStreamState::paused;
          _streamFile(_pause_script.c_str(), _current_stream->auth_type, true,
                      true);
          break;

        case ESP3DGcodeStreamState::resume:
          // add resume script to _scripts
          //_current_stream -> state = ESP3DGcodeStreamState::read_line;
          _currentPrintStream.state = ESP3DGcodeStreamState::read_line;
          _streamFile(_resume_script.c_str(), _current_stream->auth_type, true);
          break;

        case ESP3DGcodeStreamState::abort:
          // add abort script to _scripts and end stream
          //_current_stream -> state = ESP3DGcodeStreamState::end; // Does this
          // need to apply only to currentPrintStream?
          _currentPrintStream.state = ESP3DGcodeStreamState::end;
          _streamFile(_stop_script.c_str(), _current_stream->auth_type, true);
          break;

        case ESP3DGcodeStreamState::paused:  // might be worth updating the ring
                                             // buffer here
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
        // should only be used with wait_for_ack esp3d_log("Wait"); vTaskDelay?
        break;

      case ESP3DGcodeHostState::wait_for_ack:
        if (!_awaitingAck) {
          _current_state = _next_state;
          _next_state = ESP3DGcodeHostState::idle;
        } else {
          if (_current_stream != nullptr) {
            if (isFileStream(_current_stream)) {
              _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);
            }
          }
          if (esp3d_hal::millis() - _startTimeout > _timeoutInterval) {
            _current_state = ESP3DGcodeHostState::error;  // unhandled
            esp3d_log_e("Timeout waiting for ok");
          }
        }
        break;

      case ESP3DGcodeHostState::send_gcode_command: {
        if (!_awaitingAck) {  // if else shouldn't really be necessary here, as
                              // it should end up in the wait for ack state.
                              // just a precaution for now, remove later.
          esp3d_log("Sending to output client: %s", &_currentCommand[0]);
          uart_write_bytes(ESP3D_SERIAL_PORT, &_currentCommand,
                           strlen((char*)&(_currentCommand[0])));
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
        ESP3DMessage* msg = newMsg(
            ESP3DClientType::stream, ESP3DClientType::command,
            (uint8_t*)(&(_currentCommand[0])),
            strlen((char*)&(_currentCommand[0])), _current_stream->auth_type);
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

  _currentPrintStream = ESP3DGcodeFileStream{};
  _current_stream = nullptr;
  _scripts.clear();
}
#endif  // ESP3D_GCODE_HOST_FEATURE