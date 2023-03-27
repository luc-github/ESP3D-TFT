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
#if ESP3D_USB_SERIAL_FEATURE
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


// Macro behaviour:
// to be executed regardless of streaming state (ie paused)
// to be executed before resuming with file stream



ESP3DGCodeHostService gcodeHostService;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout
#define ESP3D_GCODE_HOST_TASK_SIZE 4096
#define ESP3D_GCODE_HOST_TASK_PRIORITY 5
#define ESP3D_GCODE_HOST_TASK_CORE 1
#define ESP3D_GCODE_HOST_PORT 0

#define HOST_PAUSE_SCRIPT "SD/Scripts/Pause.gco"
#define HOST_RESUME_SCRIPT "SD/Scripts/Resume.gco"
#define HOST_ABORT_SCRIPT "SD/Scripts/Abort.gco"

//#define ESP_HOST_OK_TIMEOUT 60000
//#define ESP_HOST_BUSY_TIMEOUT 10000



// main task
static void esp3d_gcode_host_task(void* pvParameter) {
  (void)pvParameter;
  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));
    gcodeHostService.handle();
  }
  vTaskDelete(NULL);
}

bool ESP3DGCodeHostService::stream(const char* file, ESP3DAuthenticationLevel auth_type, bool executeAsMacro, bool executeFirst) {

  esp3d_log("Processing file: %s, with authentication level=%d", file, static_cast<uint8_t>(auth_type));
  //ESP3DGcodeStream newStream;
  ESP3DGcodeHostFileType type = _getStreamType(file);
  //newStream.currentCommand = (uint8_t *)malloc(ESP_GCODEHOST_COMMAND_LINE_BUFFER); //if it's fixed size, it should be 

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
      //ESP3DGcodeStream newStream;
      //_currentPrintStream = newStream;
      //_currentPrintStream.fileBuffer = (char *)malloc(FILE_BUFFER_LENGTH); //should probably be initialized with struct.
      memset(&(_currentPrintStream.fileBuffer), '\0', FILE_BUFFER_LENGTH);
      _currentPrintStream.chunksize = FILE_BUFFER_LENGTH;
      _currentPrintStream._fileName = (char *)malloc(strlen(file)+1);
      strncpy(_currentPrintStream._fileName, file, strlen(file)+1);
      _currentPrintStream.type = type;
      _currentPrintStream.id = esp3d_hal::millis();
      _currentPrintStream.auth_type = auth_type;
      _currentPrintStream.commandNumber = 0;
      _currentPrintStream.resendCommandNumber = 0;
      esp3d_log("File type: %d", static_cast<uint8_t>(_currentPrintStream.type));
      return true;
    }
      break;

    case ESP3DGcodeHostFileType::script:
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostFileType::sd_script:
#endif // ESP3D_SD_CARD_FEATURE
    {
      if (executeFirst){ _scripts.push_front(ESP3DGcodeFileStream{}); } else { _scripts.push_back(ESP3DGcodeFileStream{}); }
      ESP3DGcodeFileStream& streamRef = executeFirst ? (ESP3DGcodeFileStream&)_scripts.front() : (ESP3DGcodeFileStream&)_scripts.back();
      memset(&(streamRef.fileBuffer), '\0', FILE_BUFFER_LENGTH);
      streamRef.chunksize = FILE_BUFFER_LENGTH;
      streamRef._fileName = (char *)malloc(strlen(file)+1);
      strncpy(streamRef._fileName, file, strlen(file)+1);
      streamRef.type = type;
      streamRef.id = esp3d_hal::millis();
      streamRef.auth_type = auth_type;
      streamRef.commandNumber = 0;
      streamRef.resendCommandNumber = 0;
      esp3d_log("File type: %d", static_cast<uint8_t>(streamRef.type));
      return true;
    }
      break;

    case ESP3DGcodeHostFileType::single_command:
    case ESP3DGcodeHostFileType::multiple_commands:
    {
      if (executeFirst){ _scripts.push_front(ESP3DGcodeCommandStream{}); } else { _scripts.push_back(ESP3DGcodeCommandStream{}); }
      ESP3DGcodeCommandStream& streamRef = executeFirst?(ESP3DGcodeCommandStream&)_scripts.front() : (ESP3DGcodeCommandStream&)_scripts.back();
      streamRef.commandBuffer = (char *)malloc(strlen(file)+1);
      strncpy(streamRef.commandBuffer, file, strlen(file)+1);
      streamRef.type = type;
      streamRef.id = esp3d_hal::millis();
      streamRef.auth_type = auth_type;
      esp3d_log("File type: %d", static_cast<uint8_t>(streamRef.type));
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
  /*
  ESP3DGcodeStream* script = getCurrentStream();
  if (script) {
    script->next_state = ESP3DGcodeStreamState::abort;
    return true;
  }
  */
  return true;
}

bool ESP3DGCodeHostService::pause() {

  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, xPauseIndex);
  /*
  ESP3DGcodeStream* script = getCurrentStream();
  if (script) {
    script->next_state = ESP3DGcodeStreamState::pause;
    return true;
  }
  */
  return true;
}

bool ESP3DGCodeHostService::resume() {

  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, xResumeIndex);
  /*
  ESP3DGcodeStream* script = getCurrentStream();
  if (script && script->state == ESP3DGcodeStreamState::paused) {
    script->state = ESP3DGcodeStreamState::resume;
    return true;
  }
  */
  return false;
}

bool ESP3DGCodeHostService::_isAck(const char* cmd) { //{ return false; }
  if ((strstr(cmd, "ok") != nullptr)){
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isBusy(const char* cmd) { //{ return false; }
  if ((strstr(cmd, "busy:") != nullptr)){
    return true;
  }
  return false;
}

bool ESP3DGCodeHostService::_isError(const char* cmd) { //{ return false; }
  return false; //need info on printer error formats
}

uint64_t ESP3DGCodeHostService::_resendCommandNumber(const char* cmd) {
  char* p = nullptr;
  if (((p = strstr(cmd, "resend:")) != nullptr)){
    return atoi(p+7);
  }
  return 0;
}

bool ESP3DGCodeHostService::_isEmergencyParse(const char* cmd) { //{ return false; }
  if ((strstr(cmd, "M112") != nullptr) || (strstr(cmd, "M108") != nullptr)){
    return true;
  }
  return false;
}

//bool ESP3DGCodeHostService::_isAckNeeded() { return false; }
bool ESP3DGCodeHostService::_startStream(ESP3DGcodeStream* _stream) {
  //function may not be needed after stream()
  //nothing to be done if command stream
  if (isFileStream(_stream)) {
    if(_openFile((ESP3DGcodeFileStream*) _stream)){
      return true;
    } else {
      return false;
    }
  }
  return true;
}

bool ESP3DGCodeHostService::_endStream(ESP3DGcodeStream* _stream) {//{ return false; }
//need to do if in list, destroy, else clear currentprintstream
  if(isFileStream(_stream)) {
    free(((ESP3DGcodeFileStream*)_stream) -> _fileName);
    memset(((ESP3DGcodeFileStream*)_stream) -> fileBuffer, '\0' , FILE_BUFFER_LENGTH);
    ((ESP3DGcodeFileStream*)_stream) -> commandNumber = 0;
    ((ESP3DGcodeFileStream*)_stream) -> resendCommandNumber = 0;
    fclose((((ESP3DGcodeFileStream*)_stream) -> streamFile));
  } else if (isCommandStream(_stream)) {
    free(((ESP3DGcodeCommandStream*)_stream) -> commandBuffer);
  }
  *_stream = (ESP3DGcodeStream){};

  return true;
}

bool ESP3DGCodeHostService::_readNextCommand(ESP3DGcodeStream* _stream) {

 /////////////APPROACH #1 ----- Read file in chunks of size FILE_BUFFER_LENGTH, update at end of chunk - effect: Fewer file reads of longer duration
 /////////////APPROACH #2 ----- Read file in chunks of size FILE_BUFFER_LENGTH, update after every command sent if FS/SD is available - effect: File read of size FILE_BUFFER_LENGTH for every command, regardless of size
 /////////////----------------- unless other file operation is in progress. In which case, buffer is present to provide time for FS/SD to become available again.
 /////////////APPROACH #3 ----- Only read the contents of the current line - effect: File read for every line, the size of the current line. Effect of blocking SD/FS.

  if(isFileStream(_stream)) {
    ESP3DGcodeFileStream& streamRef = (ESP3DGcodeFileStream&)*_stream;
    char c = (char) 0;
    do {
      if ( streamRef.bufferPos >= streamRef.chunksize) { //if we reach the end of the read buffer, read another chunk of the file. (FS/SD behaviour should be looked into to assess performance and if FS/SD mutexing is required).
        if (streamRef.chunksize == FILE_BUFFER_LENGTH) { //if it's a full buffer, there's likely more data, read another chunk.
          memset(&(streamRef.fileBuffer), 0, FILE_BUFFER_LENGTH);
          streamRef.bufferPos = 0;
          streamRef.chunksize = fread(&(streamRef.fileBuffer), 1, FILE_BUFFER_LENGTH, (streamRef.streamFile));
          if (streamRef.chunksize == 0) {
            //add a file size vs processed check here.
            return false;
          }
          if (streamRef.chunksize != FILE_BUFFER_LENGTH){
            //Do some checks to make sure it's not due to a read error, and attempt to rectify if so.
          }
        } else { //else if it wasn't a full buffer, we've finished the file. (or there's been an error, add a file size vs processed check here ^)
          return false;
        }
      }

      c = streamRef.fileBuffer[streamRef.bufferPos];
      streamRef.bufferPos++;
      streamRef.processedSize++;
      while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
        c = streamRef.fileBuffer[streamRef.bufferPos];
        streamRef.bufferPos++;
        streamRef.processedSize++;
      }
      while (c == ';'){ // while its a full line comment, read on to the next line
        c = streamRef.fileBuffer[streamRef.bufferPos];
        streamRef.bufferPos++;
        streamRef.processedSize++;
        while (!(isEndLineEndFile(c))){ //skim to end of line
          c = streamRef.fileBuffer[streamRef.bufferPos];
          streamRef.bufferPos++;
          streamRef.processedSize++;
        }
        while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
          c = streamRef.fileBuffer[streamRef.bufferPos];
          streamRef.bufferPos++;
          streamRef.processedSize++;
        }
      }

      while (!(isEndLineEndFile(c) || (streamRef.processedSize >= streamRef.totalSize))){ // while not end of line or end of file
        if (c == ';'){ //reached a comment, skip to next line
          while (!(isEndLineEndFile(c))){ //skim to end of line
            c = streamRef.fileBuffer[streamRef.bufferPos];
            streamRef.bufferPos++;
            streamRef.processedSize++;
          }
        } else { //no comment yet, read into command
          strcat(&(streamRef.currentCommand[0]), &c);
          c = streamRef.fileBuffer[streamRef.bufferPos];
          streamRef.bufferPos++;
          streamRef.processedSize++;
        }
      } //need to check if it's the end of buffer - if so, do read line again, appending to the current command
      
    } while(!isEndLine(c));

  } else if (isCommandStream(_stream)) {
    ESP3DGcodeCommandStream& streamRef = (ESP3DGcodeCommandStream&)*_stream;
    char c = (char) 0;
    if (streamRef.bufferPos >= strlen(streamRef.commandBuffer)) { //if we reach the end of the buffer, the command stream is done
      return false;
    }

    c = streamRef.commandBuffer[streamRef.bufferPos];
    streamRef.bufferPos++;
    streamRef.processedSize++;
    while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
      c = streamRef.commandBuffer[streamRef.bufferPos];
      streamRef.bufferPos++;
      streamRef.processedSize++;
    }
    while (c == ';'){ // while its a full line comment, read on to the next line
      c = streamRef.commandBuffer[streamRef.bufferPos];
      streamRef.bufferPos++;
      streamRef.processedSize++;
      while (!(isEndLineEndFile(c))){ //skim to end of line
        c = streamRef.commandBuffer[streamRef.bufferPos];
        streamRef.bufferPos++;
        streamRef.processedSize++;
      }
      while (isWhiteSpace(c)){ //ignore any leading spaces and empty lines
        c = streamRef.commandBuffer[streamRef.bufferPos];
        streamRef.bufferPos++;
        streamRef.processedSize++;
      }
    }

    while (!(isEndLineEndFile(c) || (streamRef.processedSize >= streamRef.totalSize))){ // while not end of line or end of file
      if (c == ';'){ //reached a comment, skip to next line
        while (!(isEndLineEndFile(c))){ //skim to end of line
          c = streamRef.commandBuffer[streamRef.bufferPos];
          streamRef.bufferPos++;
          streamRef.processedSize++;
        }
      } else { //no comment yet, read into command
        strcat(&(streamRef.currentCommand[0]), &c);
        c = streamRef.commandBuffer[streamRef.bufferPos];
        streamRef.bufferPos++;
        streamRef.processedSize++;
      }
    } 

  }

  if (strlen(&(_stream -> currentCommand[0])) == 0) {
    return false;
  } else {
    return true;
  }

  //////-------------------------------------------------------------------------------------
/*
  _processedSize++;
  _currentPosition++; 
  char c = (char)fileHandle.read(); //does casting need to be explicit? probably for the best for now at least
  while (((c =='\n') || (c =='\r') || (c == ' '))){ //ignore any leading spaces and empty lines
    _processedSize++;
    _currentPosition++;
    c = (char)fileHandle.read();
  }
  while (c == ';'){ // while its a full line comment, read on to the next line
    _processedSize++;
    _currentPosition++;
    c = (char)fileHandle.read();
    while (!((c =='\n') || (c =='\r') || (c == 0) || (c == -1))){ //skim to end of line
      _processedSize++;
      _currentPosition++;
      c = (char)fileHandle.read();
    }
    while (((c =='\n') || (c =='\r') || (c == ' '))){ //ignore any leading spaces and empty lines
      _processedSize++;
      _currentPosition++;
      c = (char)fileHandle.read();
    }
  }

  while (!((c == '\n') || (c =='\r') || (c == 0) || (c == -1) || (_currentPosition > _totalSize))){ // while not end of line or end of file
    if (c == ';'){ //reached a comment, skip to next line
      while (!((c =='\n') || (c =='\r') || (c == 0) || (c == -1))){ //skim to end of line
        _processedSize++;
        _currentPosition++;
        c = (char)fileHandle.read();
      }
    } else { //no comment yet, read into command
      _currentCommand += c;
      _processedSize++;
      _currentPosition++;
      c = (char)fileHandle.read();
    }
  }
  //}
  ///////////----------------------------------------------------------------------------------------
  */
  //return true;

} //return false at end of file/script.

bool ESP3DGCodeHostService::_gotoLine(uint64_t line){

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
    esp3d_log("Streaming command(s): %s", (char*)(rx->data));
    stream((char*)(rx->data), rx->authentication_level);
  } else if (rx->origin != ESP3DClientType::command) {
    esp3d_log("Stream got the response: %s", (char*)(rx->data));
    _parseResponse(rx);
  }
  deleteMsg(rx);
  return true;
}

bool _openFile(ESP3DGcodeFileStream* _stream) {
  esp3d_log("File name is %s", _stream -> _fileName);
  if (globalFs.accessFS((char*)(_stream -> _fileName))) {
    if (globalFs.exists((char*)(_stream -> _fileName))) {
      esp3d_log("File exists");
      //_stream -> streamFile = globalFs.open(_stream -> _filename,"r");
      _stream -> streamFile = fopen(_stream -> _fileName,"r");
      if((_stream -> streamFile) != nullptr) return true;
    }
  }
  return false;
}

//needs reworking for however states end up
ESP3DGcodeHostState ESP3DGCodeHostService::getState() {
  ESP3DGcodeStream* stream = getCurrentStream();
  if (stream) {
    if (stream->state == ESP3DGcodeStreamState::paused)
      return ESP3DGcodeHostState::paused;

    return ESP3DGcodeHostState::processing;
  }
  return ESP3DGcodeHostState::wait;
}

ESP3DGcodeHostError ESP3DGCodeHostService::getErrorNum() {
  return ESP3DGcodeHostError::no_error;
}

ESP3DGcodeStream* ESP3DGCodeHostService::getCurrentStream(
    ESP3DGcodeHostFileType type) {
  if (type == ESP3DGcodeHostFileType::active) return &_currentPrintStream;

  esp3d_log("There are currently %d scripts", _scripts.size());
  for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
    esp3d_log("Script type: %d", static_cast<uint8_t>(script->type));
    
    if ((type == ESP3DGcodeHostFileType::filesystem &&
         (script->type == ESP3DGcodeHostFileType::sd_card ||
          script->type == ESP3DGcodeHostFileType::filesystem)) ||
        (type == script->type)) {
      return &(*script);
    }
  }
  esp3d_log("No script found");
  return nullptr;
}

ESP3DGCodeHostService::ESP3DGCodeHostService() {
  _started = false;
  _xHandle = NULL;
  _current_stream = NULL;
}
ESP3DGCodeHostService::~ESP3DGCodeHostService() { end(); }

void ESP3DGCodeHostService::process(ESP3DMessage* msg) {
  esp3d_log("Add message to queue");
  if (msg->origin != ESP3DClientType::serial) {
    if (!addTxData(msg)) {
      flush();
      if (!addTxData(msg)) {
        esp3d_log_e("Cannot add msg to client queue");
        deleteMsg(msg);
      }
    } else {
      flush();
    }
  } else {
    if (!addRxData(msg)) {
      flush();
      if (!addRxData(msg)) {
        esp3d_log_e("Cannot add msg to client queue");
        deleteMsg(msg);
      }
    } else {
      flush();
    }
  }
}

inline bool ESP3DGCodeHostService::_isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}

inline bool ESP3DGCodeHostService::_isWhiteSpace(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
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
//----------------------------------------------------------------------------------------
bool ESP3DGCodeHostService::pushMsgToRxQueue(const uint8_t* msg, size_t size) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
      newMsgPtr->authentication_level = ESP3DAuthenticationLevel::user;
      newMsgPtr->origin = ESP3DClientType::stream;
      if (!addRxData(newMsgPtr)) {
        // delete message as cannot be added to the queue
        //ESP3DClient::deleteMsg(newMsgPtr);
        deleteMsg(newMsgPtr);
        esp3d_log_e("Failed to add message to rx queue");
        return false;
      }
    } else {
      // delete message as cannot be added partially filled to the queue
      free(newMsgPtr);
      esp3d_log_e("Message creation failed");
      return false;
    }
  } else {
    esp3d_log_e("Out of memory!");
    return false;
  }
  return true;
}
//---------------------------------------------------------------------------------------------
ESP3DGcodeHostFileType ESP3DGCodeHostService::_getStreamType(const char* file) { //maybe this can also check for invalid file extensions if not done elsewhere
  if (file[0] == '/') {
    if (strstr(file, "/sd/") == file) {
#if ESP3D_SD_CARD_FEATURE
      return ESP3DGcodeHostFileType::sd_card;
#else
      return ESP3DGcodeHostFileType::filesystem;
      //return ESP3DGcodeHostFileType::invalid; //maybe should return error?
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

// we can only have one fs/sd script at a time - yes (but could be changed later)
// we can only have one multiple commands script at a time - no, easy enough to queue multiple command scripts, but may be smart to have clear queue function. also would work well for pause and immediate resume - won't have to wait for pause to complete.
// there are no limit to single command scripts - yes
// the parsing go to each commands type
// 1 - File commands -> next is when ack is received
// 2 - Unique commands (macro / commands when streaming) -> next is when ack is
// received 3 - Multiple commands separated by `;` (macro) -> next is when all
// commands are processed and acked
// ^^^ Not sure I understand? 


// ESP700 can add script/commands to the queue - can we have an argument for (file)macro? or maybe just ESP70* for stream macro?
// ESP701 (action=pause/resume/abort)  monitor/control Multiple commands
// streaming ESP702 (action=pause/resume/abort)monitor/control file (SD/FS)
// streaming

void ESP3DGCodeHostService::_handle() {
  if (_started) {
    //if there's anything in the _scripts list, we should execute scripts before continuing with the file stream.
    if(ulTaskNotifyTake(pdTRUE, 0)) {
      if(ulTaskNotifyTakeIndexed(xPauseIndex, pdTRUE, 0)){
        _currentPrintStream.state = ESP3DGcodeStreamState::pause;
      }
      if(ulTaskNotifyTakeIndexed(xResumeIndex, pdTRUE, 0)){
        if(_currentPrintStream.state == ESP3DGcodeStreamState::paused) {
          _currentPrintStream.state = ESP3DGcodeStreamState::resume;
        } else if (_currentPrintStream.state == ESP3DGcodeStreamState::pause) {
          _currentPrintStream.state = ESP3DGcodeStreamState::read_line;
        }
      }
      if(ulTaskNotifyTakeIndexed(xResumeIndex, pdTRUE, 0)){
        _currentPrintStream.state = ESP3DGcodeStreamState::abort;
      }
    }
    if (!_scripts.empty()) {
      _current_stream = (ESP3DGcodeStream*)&(_scripts.front());
       esp3d_log("Processing %d scripts", _scripts.size()); //Add an if to give filename if fs or sd script
    } else if (_currentPrintStream._fileName != nullptr) { //if no scripts, and we're currently streaming a print, execute print stream
      _current_stream = &_currentPrintStream;
      esp3d_log("Processing file: %s", ((char*)(((ESP3DGcodeFileStream*)(_current_stream))->_fileName)));
    } else {
      _current_stream = nullptr;
    }

    if (_current_stream != nullptr){
      switch (_current_stream -> state){

        case ESP3DGcodeStreamState::end:
          _endStream(_current_stream);
          if((_current_stream -> type == ESP3DGcodeHostFileType::filesystem ) || (_current_stream -> type == ESP3DGcodeHostFileType::sd_card )){
            //ESP3DGcodeFileStream clearStream;
            //_currentPrintStream = clearStream;
            *_current_stream = (ESP3DGcodeFileStream){};
            //free(_currentPrintStream._fileName); //<---- should be in endStream()
          } else {
            
            _scripts.erase(_scripts.begin());

          }
          break;

        case ESP3DGcodeStreamState::start:
          _startStream(_current_stream);
          __attribute__((fallthrough));
        case ESP3DGcodeStreamState::read_line:
          if (strlen(&(_current_stream -> currentCommand[0])) == 0) {
            if(_readNextCommand(_current_stream)) {
              if (esp3dCommands.is_esp_command((uint8_t*)(&(_current_stream -> currentCommand)), strlen((&(_current_stream -> currentCommand[0]))))){
                next_state = state; //if waiting_for_ack we can send this and check on ack after we send any esp commands
                state = ESP3DGcodeHostState::send_esp_command;
              } else {
                //add line no and checksum if appropriate
                if (state == ESP3DGcodeHostState::wait_for_ack){
                  next_state = ESP3DGcodeHostState::send_gcode_command;
                } else {
                  state = ESP3DGcodeHostState::send_gcode_command;
                }
              }
            } else { //returns false at end of file/script
              //state = ESP3DGcodeHostState::wait;
              _current_stream -> state = ESP3DGcodeStreamState::end;
            }
          }
          break;

        case ESP3DGcodeStreamState::pause:
          // add pause script to _scripts
          stream(HOST_PAUSE_SCRIPT, _current_stream -> auth_type, true, true);
          _current_stream -> state = ESP3DGcodeStreamState::paused;
          break;

        case ESP3DGcodeStreamState::resume:
          // add resume script to _scripts
          stream(HOST_RESUME_SCRIPT, _current_stream -> auth_type, true);
          if (strlen((&(_current_stream -> currentCommand[0]))) == 0){
            _current_stream -> state = ESP3DGcodeStreamState::read_line;
          } else {
            _current_stream -> state = ESP3DGcodeStreamState::wait_for_send;
          }
          break;

        case ESP3DGcodeStreamState::abort:
          // add abort script to _scripts and end stream
          _current_stream -> state = ESP3DGcodeStreamState::end;
          stream(HOST_ABORT_SCRIPT, _current_stream -> auth_type, true);
          break;

        case ESP3DGcodeStreamState::paused:
          state = ESP3DGcodeHostState::wait; //may need renaming to better reflect current state of stream
          break;

        default:
          break;
      }
    }

    switch (state){

      case ESP3DGcodeHostState::wait:
        //do nothing, either no stream, or stream is paused
        //state = next_state; //shouldn't be necessary, next_state should only be used with wait_for_ack
        break;

      case ESP3DGcodeHostState::wait_for_ack:
        if (!_awaitingAck){
          state = next_state;
          next_state = ESP3DGcodeHostState::wait;
        } else {
          if (esp3d_hal::millis() - _startTimeout > _timeoutInterval){
            state = ESP3DGcodeHostState::error;
            esp3d_log_e("Timeout waiting for ok");
          }
        }
        break;

      case ESP3DGcodeHostState::send_gcode_command:
      {
        if (!_awaitingAck){ //if else shouldn't really be necessary here, as it should end up in the wait for ack state. just a precaution for now, remove later.
          ESP3DMessage* msg = newMsg(ESP3DClientType::stream, ESP3DClientType::serial, (uint8_t*)(&(_current_stream -> currentCommand)), strlen(&(_current_stream -> currentCommand[0])), _current_stream -> auth_type);
          //esp3dCommands.process(msg);// may just dispatch to serial
          serialClient.process(msg); // is there any harm in sending messages directly to the client? 
          _startTimeout = esp3d_hal::millis();
          _awaitingAck = true;
          //_current_stream->currentCommand = "";
          memset((_current_stream->currentCommand), 0, ESP_GCODEHOST_COMMAND_LINE_BUFFER);
          state = ESP3DGcodeHostState::wait_for_ack;
          next_state = ESP3DGcodeHostState::wait;
        } else {
          state = ESP3DGcodeHostState::wait_for_ack;
        }
        break;
      }

      case ESP3DGcodeHostState::send_esp_command:
      {
        ESP3DMessage* msg = newMsg(ESP3DClientType::stream, ESP3DClientType::command, (uint8_t*)(&(_current_stream -> currentCommand)), strlen(*&(_current_stream -> currentCommand)), _current_stream -> auth_type);
        esp3dCommands.process(msg);
        //*(_current_stream->currentCommand) = "";
        memset((_current_stream->currentCommand), 0, ESP_GCODEHOST_COMMAND_LINE_BUFFER);
        state = next_state;
        next_state = ESP3DGcodeHostState::wait;
        break;
      }

      default:

        break;
    }
    _current_stream = nullptr;

    while(getRxMsgsCount() > 0) {
      ESP3DMessage * msg = popRx();
      if (msg) {
          _processRx(msg);
      }
    }
  }
}

void ESP3DGCodeHostService::flush() { //should only be called when no handle task is running
  uint8_t loopCount = 10;
  while (loopCount && ((getRxMsgsCount() > 0) || (getTxMsgsCount() > 0))) { // if it's more than 1, we have issues.
    // esp3d_log("flushing Tx messages");
    loopCount--;
    handle();
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