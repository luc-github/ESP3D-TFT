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
//#define ESP3D_GCODE_HOST_FEATURE
#ifdef ESP3D_GCODE_HOST_FEATURE
#include "esp3d_gcode_host_service.h"

#include <stdio.h>

#include "serial/esp3d_serial_client.h"
#if ESP3D_USB_SERIAL_FEATURE //###################################################### <Make sure nothing special needs doing for this
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // #if ESP3D_USB_SERIAL_FEATURE

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "filesystem/esp3d_globalfs.h"
#include "filesystem/esp3d_flash.h"

#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h" // just to hide error, will need changing along with serial write uart_write_*


#include "esp32/rom/crc.h"


// Macro behaviour:
// to be executed regardless of streaming state (ie paused)
// to be executed before resuming with file stream



ESP3DGCodeHostService gcodeHostService;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout
#define ESP3D_GCODE_HOST_TASK_SIZE 4096
#define ESP3D_GCODE_HOST_TASK_PRIORITY 2
#define ESP3D_GCODE_HOST_TASK_CORE 1
#define ESP3D_GCODE_HOST_PORT 0

#define HOST_PAUSE_SCRIPT "M125\n"
#define HOST_RESUME_SCRIPT "M108\n"
#define HOST_ABORT_SCRIPT "G91\nG0 Z5\nG90\nG28 X Y\nM104 S0\nM140 S0\nM107\nM84\n"

//Comment out above and use the following for scripts in files
/*
#define HOST_PAUSE_SCRIPT "/sd/Scripts/Pause.gco"
#define HOST_RESUME_SCRIPT "/sd/Scripts/Resume.gco"
#define HOST_ABORT_SCRIPT "/sd/Scripts/Abort.gco"
*/

//#define ESP_HOST_OK_TIMEOUT 60000
//#define ESP_HOST_BUSY_TIMEOUT 10000



// main task
static void esp3d_gcode_host_task(void* pvParameter) {
  (void)pvParameter;
  while (1) {
    /* Delay */

    vTaskDelay(pdMS_TO_TICKS(10)); //Why is this necessary, and why does it have to be at least 10? Better to yield in Handle? Task handling needs looking over I think
    gcodeHostService._handle();
  }
  vTaskDelete(NULL);
}

ESP3DGcodeHostFileType ESP3DGCodeHostService::_getStreamType(const char* file) { //maybe this can also check for invalid file extensions if not done elsewhere
  if (file[0] == '/') {
    if (strstr(file, "/sd/") == file) {
#if ESP3D_SD_CARD_FEATURE
      return ESP3DGcodeHostFileType::sd_card;
#else
      //return ESP3DGcodeHostFileType::filesystem;
      return ESP3DGcodeHostFileType::invalid; //maybe should return error?
#endif  // ESP3D_SD_CARD_FEATURE
    } else {
      return ESP3DGcodeHostFileType::filesystem;
    }
  } else {
    if (strstr(file, "\n") != nullptr) { //can we use \n instead of ;?
      return ESP3DGcodeHostFileType::multiple_commands;
    } else {
      if (strlen(file) != 0)
        return ESP3DGcodeHostFileType::single_command;
      else
        return ESP3DGcodeHostFileType::invalid;
    }
  }
}

bool ESP3DGCodeHostService::_Stream(const char* file, ESP3DAuthenticationLevel auth_type, bool executeAsMacro, bool executeFirst) {

  esp3d_log("Processing stream request: %s, with authentication level=%d", file, static_cast<uint8_t>(auth_type)); //Need to implement authorisation check here
  ESP3DGcodeHostFileType type = _getStreamType(file);
  esp3d_log("File type: %d", static_cast<uint8_t>(type));

  if (executeAsMacro) {
    if (type == ESP3DGcodeHostFileType::filesystem) {
      type = ESP3DGcodeHostFileType::script;
#if ESP3D_SD_CARD_FEATURE
    } else if (type == ESP3DGcodeHostFileType::sd_card) {
      type = ESP3DGcodeHostFileType::sd_script;
#endif // ESP3D_SD_CARD_FEATURE
    }
  }

  switch (type) {
    case ESP3DGcodeHostFileType::filesystem:
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostFileType::sd_card:
#endif // ESP3D_SD_CARD_FEATURE
    {
      if ( getCurrentStream() != nullptr) return false;
      //_currentPrintStream.chunksize = RING_BUFFER_LENGTH; // <- Review this
      for (int attempt = 0; attempt < 5; attempt++){
        _currentPrintStream._fileName = (uint8_t *)pvPortMalloc(sizeof(char)*(strlen(file)+1)); //Need to limit memory usage -- Does task memory allocation do this?
        if(_currentPrintStream._fileName != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log_w("Memory allocation attempt %d failed", attempt+1);
      }
      if (_currentPrintStream._fileName == nullptr){
        esp3d_log_e("Memory allocation failed");
        vPortFree(*(executeFirst? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase((executeFirst? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else { 
        esp3d_log("Memory allocated at address: %d", (unsigned int)(_currentPrintStream._fileName));
      }
      strncpy((char*)_currentPrintStream._fileName, file, strlen(file)+1);
      _currentPrintStream.type = type;
      _currentPrintStream.id = esp3d_hal::millis();
      _currentPrintStream.auth_type = auth_type;
      _currentPrintStream.commandNumber = 0;
      _currentPrintStream.resendCommandNumber = 0;
      _currentPrintStream.state = ESP3DGcodeStreamState::start;
      esp3d_log("Stream type: %d", static_cast<uint8_t>(_currentPrintStream.type));

      //send line number reset command if commencing a print stream.
      const char lineno_reset[] = "M110 N0\n"; //Is there a better way to store this const?
      _Stream(lineno_reset, auth_type);

      return true;
    }
      break;

    case ESP3DGcodeHostFileType::script:
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostFileType::sd_script:
#endif // ESP3D_SD_CARD_FEATURE
    {
      if (executeFirst){ _scripts.push_front(new ESP3DGcodeFileStream()); } else { _scripts.push_back(new ESP3DGcodeFileStream()); }
      ESP3DGcodeFileStream* streamPointer = (executeFirst? ((ESP3DGcodeFileStream*)(_scripts.front())) : ((ESP3DGcodeFileStream*)(_scripts.back())));  
      for (int attempt = 0; attempt < 5; attempt++){
        streamPointer->_fileName = (uint8_t *)pvPortMalloc(sizeof(char)*(strlen(file)+1));
        if(streamPointer->_fileName != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log_w("Memory allocation attempt %d failed", attempt+1);
      }
      if (streamPointer->_fileName == nullptr){
        esp3d_log_e("Memory allocation failed");
        vPortFree(*(executeFirst? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase((executeFirst? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else {
        esp3d_log("Memory allocated at address: %d", (unsigned int)(streamPointer->_fileName));
      }
      strncpy((char*)(streamPointer->_fileName), file, strlen(file)+1);
      //streamPointer->chunksize = RING_BUFFER_LENGTH;
      streamPointer->type = type;
      streamPointer->id = esp3d_hal::millis();
      streamPointer->auth_type = auth_type;
      streamPointer->commandNumber = 0;
      streamPointer->resendCommandNumber = 0;
      streamPointer->state = ESP3DGcodeStreamState::start;
      esp3d_log("Script type: %d", static_cast<uint8_t>(streamPointer->type));
      return true;
    }
      break;

    case ESP3DGcodeHostFileType::single_command:
    case ESP3DGcodeHostFileType::multiple_commands:
    {
      if (executeFirst){ _scripts.push_front(new ESP3DGcodeCommandStream()); } else { _scripts.push_back(new ESP3DGcodeCommandStream()); }
      ESP3DGcodeCommandStream* streamPointer = (executeFirst? ((ESP3DGcodeCommandStream*)(_scripts.front())) : ((ESP3DGcodeCommandStream*)(_scripts.back())));
      for (int attempt = 0; attempt < 5; attempt++){
        streamPointer->commandBuffer = (uint8_t *)pvPortMalloc(sizeof(char)*(strlen(file)+1));
        if(streamPointer->commandBuffer != nullptr) break;
        vTaskDelay(pdMS_TO_TICKS(10));
        esp3d_log("malloc attempt %d failed", attempt+1);
      }
      if (streamPointer->commandBuffer == nullptr){
        esp3d_log("malloc failed");
        vPortFree(*(executeFirst? _scripts.begin() : prev(_scripts.end())));
        _scripts.erase((executeFirst? _scripts.begin() : prev(_scripts.end())));
        return false;
      } else {
        esp3d_log("malloc'd");
        esp3d_log("Buffer address: %d", (unsigned int)(streamPointer->commandBuffer));
      }
      esp3d_log("Command length: %d", (unsigned int)strlen(file));
      strncpy((char*)streamPointer->commandBuffer, file, strlen(file)+1);
      esp3d_log("copied");
      streamPointer->type = type;
      streamPointer->id = esp3d_hal::millis();
      streamPointer->auth_type = auth_type;
      streamPointer->state = ESP3DGcodeStreamState::start;
      esp3d_log("id: %d", (int)streamPointer->id );
      esp3d_log("File type: %d", static_cast<uint8_t>(streamPointer->type));
      return true;
    }
      break;

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
  xTaskNotifyGiveIndexed(_xHandle, xAbortIndex);
  return true;
}

bool ESP3DGCodeHostService::pause() {

  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, xPauseIndex);
  return true;
}

bool ESP3DGCodeHostService::resume() {

  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, xResumeIndex);
  return true;
}

//##################### Parsing Functions ########################

bool ESP3DGCodeHostService::_isAck(const char* cmd) {
  if ((strstr(cmd, "ok") != nullptr)){
    esp3d_log("Got ok");
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isBusy(const char* cmd) {
  if ((strstr(cmd, "busy:") != nullptr)){
    esp3d_log("Got busy");
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isError(const char* cmd) {
  return false; //need info on printer error formats
}

uint64_t ESP3DGCodeHostService::_resendCommandNumber(const char* cmd) { //we should rename this
  char* p = nullptr;
  if (((p = strstr(cmd, "resend:")) != nullptr)){
    esp3d_log("Got resend");
    return atoi(p+7);
  }
  return 0;
}

bool ESP3DGCodeHostService::_isEmergencyParse(const char* cmd) {
  if ((strstr(cmd, "M112") != nullptr) || (strstr(cmd, "M108") != nullptr)){
    return true;
  }
  return false;
}

//##################### File/Script Handling Functions ########################

bool ESP3DGCodeHostService::_openFile(ESP3DGcodeFileStream* _stream) { 
  esp3d_log("File name is %s", _stream -> _fileName);
    if (globalFs.accessFS((char*)(_stream -> _fileName))) {
    if (globalFs.exists((char*)(_stream -> _fileName))) {
      esp3d_log("File exists");
      _stream -> streamFile = globalFs.open((char*)_stream -> _fileName,"r");
      if((_stream -> streamFile) != nullptr) {
        if (_current_stream -> lineStart != 0){
          if (fseek(((ESP3DGcodeFileStream*)_current_stream) -> streamFile, (long)_current_stream -> lineStart, SEEK_SET) != 0) { //this would need adjusting if concurrrent file access is implemented (seek to buffer end instead)
            esp3d_log_e("Failed to seek to correct position in file: %s", ((ESP3DGcodeFileStream*)_current_stream) -> _fileName);
            return false;
            //ERROR HERE
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

bool ESP3DGCodeHostService::_closeFile(ESP3DGcodeFileStream* _stream) { 
  
  if (_stream->streamFile == nullptr){
    esp3d_log_e("No file to close");
    return false;
  }
  
  esp3d_log("Closing File: %s", _stream -> _fileName);
  globalFs.close((_stream -> streamFile), (char*)(_stream -> _fileName));
  globalFs.releaseFS((char*)(_stream -> _fileName));
  
  return true;
}

//Update the ring buffer with data from the stream _stream. Call whenever idle to reduce latency.
//Must be called when switching streams or if the end of the buffer is reached.
bool ESP3DGCodeHostService::_updateRingBuffer(ESP3DGcodeFileStream* _stream) {

  // _bufferSeam: This is the index of the earliest byte (end) in the buffer. 
  
  //if the correct stream is not buffered, reset and fill the buffer
  if (_bufferedStream != _stream) {
    //Set indexes and load new data
    _bufferPos = 0;
    _bufferedStream = _stream;
    _bufferFresh = true;

    if ((_bufferSeam = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH, (_stream -> streamFile))) == RING_BUFFER_LENGTH){
      _bufferSeam = 0;
    } else {
      _ringBuffer[_bufferSeam] = 0;
    }
    esp3d_log("Buffered stream switched");

  } else {

    if ((_bufferPos == _bufferSeam) && (_bufferFresh == true)){
      esp3d_log("Buffer is full");
      return false; //return, buffer is already full of fresh data
    }
    if ((_ringBuffer[_bufferSeam] == 0) || (_ringBuffer[_bufferPos] == 0)){
      esp3d_log("File finished");
      return false; //return, file finished
    }

    esp3d_log("Updating buffer from: %s", _ringBuffer);

    int empty = 0;
    int read = 0;
    //if we've reached the seam we need to read a whole buffer
    if (_bufferPos == _bufferSeam){
      empty = RING_BUFFER_LENGTH; //maybe not needed
      read = fread(_ringBuffer, sizeof(char), RING_BUFFER_LENGTH, (_stream -> streamFile));
      _bufferPos = 0;
      if (read == empty){
        _bufferSeam = 0;
        _bufferFresh = true;
        esp3d_log("Read full buffer");
      } else { // if fewer bytes are read than were requested, end of file is in the buffer - place EOF char accordingly.
        _bufferSeam = read; //0 indexing makes this the byte after last char.
        _ringBuffer[_bufferSeam] = 0; //if this is 0, it's the end of file
        esp3d_log("Read partial buffer");
      }

    //if we're after the seam do a single read to update from the seam to our position
    } else if ( _bufferPos > _bufferSeam) {
      empty = _bufferPos - _bufferSeam;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char), empty, (_stream -> streamFile));
      if (read == empty){
        _bufferSeam = _bufferPos;
        _bufferFresh = true;
      } else {
        while (read > 0){
          (_bufferSeam == RING_BUFFER_LENGTH - 1) ? _bufferSeam = 0 : _bufferSeam++;
          read--;
        }
        _ringBuffer[_bufferSeam] = 0;
      }
      esp3d_log("Updated Buffer")
    //if we're before the seam do two reads to update from seam to buffer end, and from buffer start to current position.
    } else {
      empty = RING_BUFFER_LENGTH - _bufferSeam + _bufferPos;
      read = fread(&_ringBuffer[_bufferSeam], sizeof(char), RING_BUFFER_LENGTH - _bufferSeam, (_stream -> streamFile));
      if (_bufferPos > 0) {
        read += fread(_ringBuffer, sizeof(char), _bufferPos, (_stream -> streamFile));
      }
      if (read == empty){
        _bufferSeam = _bufferPos;
        _bufferFresh = true;
      } else {
        while (read > 0){
          (_bufferSeam == RING_BUFFER_LENGTH - 1) ? _bufferSeam = 0 : _bufferSeam++;
          read--;
        }
        _ringBuffer[_bufferSeam] = 0;
      }
      esp3d_log("Updated Buffer")
    }
  }

  esp3d_log("Updated buffer to: %s", _ringBuffer);
  return true;

}

//Read a character from the ring buffer, updating the buffer as required. Returns 0 at file end.
char ESP3DGCodeHostService::_readCharFromRingBuffer(ESP3DGcodeFileStream* _stream) {
  
  //if we're at the seam, and not fresh, whole buffer is used and needs updating.
  if (_bufferFresh == true) {
    if (_bufferPos != _bufferSeam){
      _bufferFresh = false;
    }
  } else if (_bufferPos == _bufferSeam){
    if (_bufferFresh == false) _updateRingBuffer(_stream);
  }
  
  char c = _ringBuffer[_bufferPos];
  (_bufferPos == RING_BUFFER_LENGTH - 1) ? _bufferPos = 0 : _bufferPos++;
  return c; // if this returns NULL, we're at endfile
}

//Read a character from a queued script's buffer. Returns 0 at script end.
char ESP3DGCodeHostService::_readCharFromCommandBuffer(ESP3DGcodeCommandStream* _stream){
  char c = _stream -> commandBuffer[((ESP3DGcodeCommandStream*)_stream) -> bufferPos];
  _stream -> bufferPos++;
  return c;
}

//##################### Stream Handling Functions ########################

bool ESP3DGCodeHostService::_startStream(ESP3DGcodeStream* _stream) { //get rid of this, initialize size etc in _Stream()
  //function may not be needed after _Stream() <------ do this for sure.
  //nothing to be done if command stream
  _stream -> lineStart = 0;
  if (isFileStream(_stream)) {
    //if(_openFile((ESP3DGcodeFileStream*) _stream)){
    if(((ESP3DGcodeFileStream*)_stream) -> _fileName != nullptr) {
      int pos = ftell(((ESP3DGcodeFileStream*)_stream) -> streamFile) + 1; // we can get rid of this if we make sure to update the buffer after this function is called.
      fseek(((ESP3DGcodeFileStream*)_stream) -> streamFile, 0, SEEK_END);
      ((ESP3DGcodeCommandStream*)_stream) -> totalSize = ftell(((ESP3DGcodeFileStream*)_stream) -> streamFile) + 1; // +1? for size instead of end index
      fseek(((ESP3DGcodeFileStream*)_stream) -> streamFile, pos, SEEK_SET);
      esp3d_log("Size is: %d", (unsigned int)((ESP3DGcodeFileStream*)_stream) -> totalSize);
      return true;
    }
    esp3d_log_e("Stream file not currently open");
    return false;
  } else if (isCommandStream(_stream)) {
    ((ESP3DGcodeCommandStream*)_stream) -> totalSize = strlen((char*)((ESP3DGcodeCommandStream*)_stream) -> commandBuffer);
    esp3d_log("Size is: %d", (unsigned int)((ESP3DGcodeCommandStream*)_stream) -> totalSize);
    return true;
  }
  esp3d_log_e("Unknown stream type");
  return false;
}

//End the stream, freeing the memory allocated to in _Stream().
bool ESP3DGCodeHostService::_endStream(ESP3DGcodeStream* _stream) {
  esp3d_log("Freeing stream buffers");
  if(isFileStream(_stream)) {
    _closeFile((ESP3DGcodeFileStream*)_stream); //Behaviour may need altering depending on consecutive storage access
    vPortFree(((ESP3DGcodeFileStream*)_stream) -> _fileName);
    //memset(_ringBuffer, 0 , RING_BUFFER_LENGTH); //shouldn't be necessary
    if (_stream == &_currentPrintStream){
      *_stream = (ESP3DGcodeFileStream){}; //Take a look over this, I don't think we need all of these lines
      _currentPrintStream._fileName = nullptr;
      _bufferedStream = nullptr;
    } else {
      esp3d_log("Erasing script from queue");
      vPortFree(*(_scripts.begin()));
      _scripts.erase(_scripts.begin());
    }
  } else if (isCommandStream(_stream)) {
    vPortFree((((ESP3DGcodeCommandStream*)_stream) -> commandBuffer));
    ((ESP3DGcodeCommandStream*)_stream) -> commandBuffer = nullptr;
    esp3d_log("Erasing script from queue");
    vPortFree(*(_scripts.begin()));
    _scripts.erase(_scripts.begin());
  }
  esp3d_log("Freed");

  return true;
}

bool ESP3DGCodeHostService::_readNextCommand(ESP3DGcodeStream* _stream) {

  _stream -> lineStart = _stream -> processedSize;
  //esp3d_log("Setting line start to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> lineStart);
  uint32_t cmdPos = 0;

  if(isFileStream(_stream)){
    /*
    if (_bufferedStream != _stream) {
    _updateRingBuffer((ESP3DGcodeFileStream*)_stream);
    }
    */
    char c = (char) 0;

    do{
      c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
      ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
        //esp3d_log_e("Setting procced to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> processedSize);

      while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
        c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
        ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
        //esp3d_log_e("Setting procced to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> processedSize);
      }
      while (c == ';'){ // while its a full line comment, skim to next line
        while (!(isEndLineEndFile(c))){
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
          ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
          //esp3d_log_e("Setting procced to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> processedSize);
        }
        while (isWhiteSpace(c)){ //ignore leading spaces and empty lines
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
          ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
          //esp3d_log_e("Setting procced to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> processedSize);
        }
      }
      _stream -> lineStart = _stream -> processedSize;
      //esp3d_log("Setting line start to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> lineStart);

      while (!(isEndLineEndFile(c) || (((ESP3DGcodeFileStream*)_stream) -> processedSize >= ((ESP3DGcodeFileStream*)_stream) -> totalSize))){ // while not end of line or end of file
        if (c == ';'){ //reached a comment, skim to next line
          while (!(isEndLineEndFile(c))){
            c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
            ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
          }
        } else { //no comment yet, read into command
          _currentCommand[cmdPos] = c;
          cmdPos++;
          c = _readCharFromRingBuffer((ESP3DGcodeFileStream*)_stream);
          ((ESP3DGcodeFileStream*)_stream) -> processedSize++;
        }
      }

    } while(!isEndLineEndFile(c));

  } else if (isCommandStream(_stream)) {
    
    esp3d_log("Is command stream");
    esp3d_log("Buffer size is: %d", strlen((char*)((ESP3DGcodeCommandStream*)_stream) -> commandBuffer));
    char c = (char) 0;
    if (((ESP3DGcodeCommandStream*)_stream) -> bufferPos >= _stream -> totalSize) { //if we reach the end of the buffer, the command stream is done
      esp3d_log("End of Buffer");
      return false;
    }
    //c = (char)((ESP3DGcodeCommandStream*)_stream) -> commandBuffer[((ESP3DGcodeCommandStream*)_stream) -> bufferPos];
    //((ESP3DGcodeCommandStream*)_stream) -> bufferPos++;

    c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
    ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
    while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
      esp3d_log("Is Whitespace");
      c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
      ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
    }
    while (c == ';'){ // while its a full line comment, read on to the next line
      c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
      ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
      while (!(isEndLineEndFile(c))){ //skim to end of line
        esp3d_log("Isn't Endline/File");
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
        ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
      }
      while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
      esp3d_log("Is Whitespace");
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
        ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
      }
    }
    esp3d_log("First char is: %c", c);
    //((ESP3DGcodeCommandStream*)_stream) -> lineStart = ((ESP3DGcodeCommandStream*)_stream) -> processedSize;
    _stream -> lineStart = _stream -> processedSize;
    esp3d_log("Setting line start to: %d", (int)((ESP3DGcodeFileStream*)_stream) -> lineStart);
    while (!(isEndLineEndFile(c) || (((ESP3DGcodeCommandStream*)_stream) -> processedSize >= ((ESP3DGcodeCommandStream*)_stream) -> totalSize))){ // while not end of line or end of file
      esp3d_log("Isn't Endline/File");
      if (c == ';'){ //reached a comment, skip to next line
        esp3d_log("Is comment");
        while (!(isEndLineEndFile(c))){ //skim to end of line
          esp3d_log("Still comment");
          c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
          ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
        }
      } else { //no comment yet, read into command
        esp3d_log("Appending %c", c);
        //strcat(_currentCommand, &c);
        _currentCommand[cmdPos] = c;
        cmdPos++;
        c = _readCharFromCommandBuffer((ESP3DGcodeCommandStream*) _stream);
        ((ESP3DGcodeCommandStream*)_stream) -> processedSize++;
      }
    } 

  }

  _currentCommand[cmdPos] = 0;
  esp3d_log("CurrCommand: %s", _currentCommand);
  //_currentCommand[cmdPos] = '\n'; //<------------------------------- needs moving after the if statement
  if (strlen((char*)_currentCommand) == 0) {
    esp3d_log("Line Empty");
    return false;
  } else {
    //_currentCommand[cmdPos] = '\n';
    //_currentCommand[cmdPos+1] = 0;
    esp3d_log("Command Read");
    return true;
  }

  return false;

} //return false at end of file/script.

uint8_t ESP3DGCodeHostService::_Checksum(const char * command, uint32_t commandSize)
{
  uint8_t checksum_val =0;
  if (command == NULL) {
      return 0;
  }
  for (uint32_t i=0; i < commandSize; i++) {
      checksum_val = checksum_val ^ ((uint8_t)command[i]);
  }
  return checksum_val;
}

//maybe rejig to use _currentCommand and cmdno (no vars necessary) //maybe absorb _Checksum() //use esp32 crc
bool ESP3DGCodeHostService::_CheckSumCommand(char* command, uint32_t commandnb)
{
  if (command[0] == 0) return false;
  std::string stringy;

  char buffer[ESP_GCODE_HOST_COMMAND_LINE_BUFFER];
  //std::copy(begin(command), end(command), )
  strcpy(buffer, command);
  //itoa(commandnb, buffer, 10);
  //buffer = *buffer;
  //stringy = "N" + (std::string)buffer + " ";

  //buffer = "N" + 
  //strcat(buffer, )
  uint8_t crc = _Checksum(command, strlen(command));
  sprintf(command, "N%u %s *%u", (unsigned int) commandnb, buffer, (unsigned int) crc);
  //command = buffer;
  //esp3d_log_e("Checksum gives: %s", command);
  //*command = "N" + c_str(buffer) + " " + command;
  //String commandchecksum = "N" + String((uint32_t)commandnb)+ " " + command;
  //commandchecksum+="*"+String(crc);
  return true;
}

bool ESP3DGCodeHostService::_gotoLine(uint64_t line){ //remember to account for M110 Nx commands
  //Not yet implemented
  return false;

}

bool ESP3DGCodeHostService::_parseResponse(ESP3DMessage* rx) { //{ return false; }
  if (_isAck((char*)(rx->data))) {
    if (_awaitingAck){
      _awaitingAck = false;
    } else {
      esp3d_log_w("Got ok but out of the query");
    }
  } else if (_isBusy((char*)(rx->data))) {
    esp3d_log("Has busy protocol, shorten timeout");
    _startTimeout = esp3d_hal::millis();
    _timeoutInterval = ESP_HOST_BUSY_TIMEOUT;
  } else if ((_currentPrintStream.resendCommandNumber = _resendCommandNumber((char*)rx->data)) != 0) {
    //_gotoLine(_currentPrintStream.resendCommandNumber);
    if (_awaitingAck){
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

  if ((rx->origin != ESP3DClientType::serial) && (rx->origin != ESP3DClientType::command)) {
    esp3d_log("Streaming: %s", (char*)(rx->data));
    _Stream((char*)(rx->data), rx->authentication_level);
  } else if (rx->origin != ESP3DClientType::command) {
    esp3d_log("Stream got the response: %s", (char*)(rx->data));
    _parseResponse(rx);
  }
  deleteMsg(rx);
  return true;
}

//needs reworking for however states end up
ESP3DGcodeHostState ESP3DGCodeHostService::getState() {
  ESP3DGcodeStream* stream = getCurrentStream();
  if (stream) {
    if (stream->state == ESP3DGcodeStreamState::paused)
      return ESP3DGcodeHostState::paused;

    return ESP3DGcodeHostState::processing;
  }
  return ESP3DGcodeHostState::idle;
}

ESP3DGcodeStream* ESP3DGCodeHostService::getCurrentStream( ///CHECK OVER THIS FUNCTION -- Not very thread safe, what is best way to send string? Msg?
    ESP3DGcodeHostFileType type) {
  //Do we care about reporting the number of terminal commands in the queue?

  if (type == ESP3DGcodeHostFileType::active) {
    if(_currentPrintStream._fileName == nullptr) {
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
  //Not yet implemented
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
    //flush();
    if (!addRxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  } else {
    //flush();
  }
  //}
}

bool ESP3DGCodeHostService::begin() {
  end();
  mutexInit();
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
      //esp3d_log("Processing %d script(s)", _scripts.size());

      if (_current_stream != (ESP3DGcodeStream*)(_scripts.front())) { //if we're not executing the front script
        esp3d_log("Switching streams");
        
        //if a different stream is loaded, close it, updating it's line position if necessary
        if (_current_stream != nullptr){
          if (_currentCommand[0] == 0){
            (_current_stream -> lineStart) = (_current_stream -> processedSize);
          } else {
            (_current_stream -> processedSize) = (_current_stream -> lineStart);
            _currentCommand[0] = 0;
            if (_current_stream == &_currentPrintStream) _currentPrintStream.commandNumber -= 1;
          }
          //if the open stream is a file, close the file
          if (isFileStream(_current_stream)) { //need to handle command stream changes too - re lineStart
            esp3d_log("closing current file stream"); //REVIEW FILE HANDLING
            _closeFile((ESP3DGcodeFileStream*)_current_stream);
          }
        }

        _current_stream = (ESP3DGcodeStream*)(_scripts.front());
        if (isFileStream(_current_stream)) {
          _openFile((ESP3DGcodeFileStream*)_current_stream);          
          _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);
        }
      } //if we're streaming the same script, no need to do anything here

    } else if (_currentPrintStream._fileName != nullptr) { //if no scripts, and we're currently streaming a print, execute print stream

      if (_current_stream != &_currentPrintStream) {
        esp3d_log("Switching streams");
        if (_current_stream != nullptr){
          if (isFileStream(_current_stream)) {
            esp3d_log("Closing current file stream at idx: %d", (int)ftell(((ESP3DGcodeFileStream*)_current_stream)->streamFile));
            _closeFile((ESP3DGcodeFileStream*)_current_stream);
          }
          if (_currentCommand[0] == 0){ // if we just sent the loaded line, update linestart, else reset processed.
            (_current_stream -> lineStart) = (_current_stream -> processedSize);
          } else {
            (_current_stream -> processedSize) = (_current_stream -> lineStart);
            _currentCommand[0] = 0;
          }
        }
        
        _current_stream = &_currentPrintStream;
        
        if (isFileStream(_current_stream)) { //unnecessary if
          esp3d_log("Opening current print stream file and reading into buffer");
          for (int attempt = 0; attempt < 5; attempt++){
            _openFile((ESP3DGcodeFileStream*)_current_stream);
            esp3d_log("fopen attempt %d", attempt+1);
            if (((ESP3DGcodeFileStream*)_current_stream) -> streamFile != nullptr) break;
            vTaskDelay(pdTICKS_TO_MS(50));
          }

          if (((ESP3DGcodeFileStream*)_current_stream) -> streamFile != nullptr) {
            esp3d_log("File Opened, addr: %d", (int)((ESP3DGcodeFileStream*)_current_stream) -> _fileName);
          } else {
            esp3d_log_e("File failed to open");            
          }

          if (_current_stream -> lineStart != 0){ //move into open file?
            esp3d_log("Seeking to line start at: %d", (int)(_current_stream -> lineStart));
            
            if (fseek(((ESP3DGcodeFileStream*)_current_stream) -> streamFile,(long) (_current_stream -> lineStart -1), SEEK_SET) != 0) { //does this need -1?
              //do error stuff
              esp3d_log_e("Failed to seek to correct line in file stream: %s", ((ESP3DGcodeFileStream*)_current_stream) -> _fileName);
            } else {
              esp3d_log("Seek completed");
            }
          }

          _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);

          esp3d_log("File buffer read completed");
        }
      }

    } else {
      esp3d_log("No Stream");
      _current_stream = nullptr;
    }

  return true;
}

void ESP3DGCodeHostService::_handle() {

  if (_started) {
    //Check for notifications, set the current stream state
    if(ulTaskNotifyTake(pdTRUE, 0)) {
      if(ulTaskNotifyTakeIndexed(xPauseIndex, pdTRUE, 0)){
        esp3d_log("Received pause notification");
        if(_currentPrintStream._fileName != nullptr){
          _currentPrintStream.state = ESP3DGcodeStreamState::pause;
        }
      }
      if(ulTaskNotifyTakeIndexed(xResumeIndex, pdTRUE, 0)){
        esp3d_log("Received resume notification");
        if(_currentPrintStream.state == ESP3DGcodeStreamState::paused) {
          _currentPrintStream.state = ESP3DGcodeStreamState::resume;
        } else if (_currentPrintStream.state == ESP3DGcodeStreamState::pause) { //if we haven't done pause actions yet, we don't need to do resume actions - just cancel the pause.
          _currentPrintStream.state = ESP3DGcodeStreamState::read_line;
        } else {
          esp3d_log_w("No paused stream - nothing to resume");
        }
      }
      if(ulTaskNotifyTakeIndexed(xAbortIndex, pdTRUE, 0)){
        esp3d_log("Received abort notification");
        if(_currentPrintStream._fileName != nullptr){
          _currentPrintStream.state = ESP3DGcodeStreamState::abort;
        }
      }
    }

    _setStream();
    
    if (_current_stream != nullptr){

      switch (_current_stream -> state){

        case ESP3DGcodeStreamState::end:
          esp3d_log("Ending Stream");
          _endStream(_current_stream);
          _current_stream = nullptr;
          break;

        case ESP3DGcodeStreamState::start:
          esp3d_log("Starting Stream");
          _startStream(_current_stream); //may be worth eliminating - only checks total file size
          _current_stream -> state = ESP3DGcodeStreamState::read_line; //Put this in _startStream()?
          __attribute__((fallthrough));

        case ESP3DGcodeStreamState::read_line:
          if (_currentCommand[0] == 0) {
            esp3d_log("Reading Line");
            if(_readNextCommand(_current_stream)) {
              if (esp3dCommands.is_esp_command((uint8_t*)(&(_currentCommand[0])), strlen((char*)(&(_currentCommand[0]))))){
                esp3d_log("Is ESP Command");
                next_state = state; //if waiting_for_ack we can send this and check on ack after we send any esp commands
                state = ESP3DGcodeHostState::send_esp_command;
                strcat((char*)_currentCommand, "\n");
              } else {
                esp3d_log("Is Gcode Command");
                //add line no and checksum if appropriate
                if (_current_stream == &_currentPrintStream){
                  _currentPrintStream.commandNumber += 1;
                  _CheckSumCommand((char *)_currentCommand, _currentPrintStream.commandNumber);                      
                }
                strcat((char*)_currentCommand, "\n");
                if (state == ESP3DGcodeHostState::wait_for_ack){
                  next_state = ESP3DGcodeHostState::send_gcode_command;
                } else {
                  state = ESP3DGcodeHostState::send_gcode_command;
                }
              }
            } else { //returns false at end of file/script
              esp3d_log("Stream Empty");
              next_state = ESP3DGcodeHostState::idle;
              _current_stream -> state = ESP3DGcodeStreamState::end;
            }
          }
          break;

        case ESP3DGcodeStreamState::pause:
          // add pause script to _scripts
          //_current_stream -> state = ESP3DGcodeStreamState::paused;
          _currentPrintStream.state = ESP3DGcodeStreamState::paused;
          _Stream(HOST_PAUSE_SCRIPT, _current_stream -> auth_type, true, true);
          break;

        case ESP3DGcodeStreamState::resume:
          // add resume script to _scripts
          //_current_stream -> state = ESP3DGcodeStreamState::read_line;
          _currentPrintStream.state = ESP3DGcodeStreamState::read_line;
          _Stream(HOST_RESUME_SCRIPT, _current_stream -> auth_type, true);
          break;

        case ESP3DGcodeStreamState::abort:
          // add abort script to _scripts and end stream
          //_current_stream -> state = ESP3DGcodeStreamState::end; // Does this need to apply only to currentPrintStream?
          _currentPrintStream.state = ESP3DGcodeStreamState::end;
          _Stream(HOST_ABORT_SCRIPT, _current_stream -> auth_type, true);
          break;

        case ESP3DGcodeStreamState::paused: //might be worth updating the ring buffer here
          state = ESP3DGcodeHostState::idle; //may need renaming to better reflect current state of stream
          break;

        default:
          break;
      }
    }

    
    //esp3d_log("Host state: %d", static_cast<uint8_t>(state));
    switch (state) {

      case ESP3DGcodeHostState::idle:
        //esp3d_log("Idling");
        //do nothing, either no stream, or stream is paused
        //state = next_state; //shouldn't be necessary, next_state should only be used with wait_for_ack
        //esp3d_log("Wait");
        //vTaskDelay?
        break;

      case ESP3DGcodeHostState::wait_for_ack:
        if (!_awaitingAck){
          state = next_state;
          next_state = ESP3DGcodeHostState::idle;
        } else {
          if (_current_stream != nullptr){
            if(isFileStream(_current_stream)){
              _updateRingBuffer((ESP3DGcodeFileStream*)_current_stream);
            }
          }
          if (esp3d_hal::millis() - _startTimeout > _timeoutInterval){
            state = ESP3DGcodeHostState::error; //unhandled
            esp3d_log_e("Timeout waiting for ok");
          }
        }
        break;

      case ESP3DGcodeHostState::send_gcode_command:
      {
        if (!_awaitingAck){ //if else shouldn't really be necessary here, as it should end up in the wait for ack state. just a precaution for now, remove later.
        esp3d_log("Sending to serial: %s", &_currentCommand[0]);
          ESP3DMessage* msg = newMsg(ESP3DClientType::stream, ESP3DClientType::serial, (uint8_t*)(&(_currentCommand[0])), strlen((char*)&(_currentCommand[0])), _current_stream -> auth_type);
          esp3dCommands.process(msg);// may just dispatch to serial
          //serialClient.process(msg); //may just access uart directly
          _startTimeout = esp3d_hal::millis();
          _awaitingAck = true;
          _currentCommand[0] = 0;
          state = ESP3DGcodeHostState::wait_for_ack;
          next_state = ESP3DGcodeHostState::idle;
        } else {
          state = ESP3DGcodeHostState::wait_for_ack;
        }
        break;
      }

      case ESP3DGcodeHostState::send_esp_command:
      {
        ESP3DMessage* msg = newMsg(ESP3DClientType::stream, ESP3DClientType::command, (uint8_t*)(&(_currentCommand[0])), strlen((char*)&(_currentCommand[0])), _current_stream -> auth_type);
        esp3dCommands.process(msg);
        _currentCommand[0] = 0;
        state = next_state;
        next_state = ESP3DGcodeHostState::idle;
        break;
      }

      default:
      //error maybe?
        break;
    }

    //should probably put this in a function

    while(getRxMsgsCount() > 0) {
      ESP3DMessage * msg = popRx();
      esp3d_log("RX popped");
      if (msg) {
          _processRx(msg);
      }
    }
  }
}

void ESP3DGCodeHostService::flush() { //should only be called when no handle task is running
  uint8_t loopCount = 10;
  while (loopCount && ((getRxMsgsCount() > 0) || (getTxMsgsCount() > 0))) {
    esp3d_log("flushing Tx messages");
    loopCount--;
    _handle();
  }
}

void ESP3DGCodeHostService::end() {
  if (_started) {

    flush(); //maybe after vTaskDelete()?
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_log("Clearing queue Tx messages");
    clearTxQueue();
    vTaskDelay(pdMS_TO_TICKS(1000));
    mutexDestroy();

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