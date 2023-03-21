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
#if ESP3D_GCODE_HOST_FEATURE
#include "esp3d_gcode_host_service.h"

#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "filesystem/esp3d_flash.h"
#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

ESP3DGCodeHostService gcodeHostService;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout
#define ESP3D_GCODE_HOST_TASK_SIZE 4096
#define ESP3D_GCODE_HOST_TASK_PRIORITY 5
#define ESP3D_GCODE_HOST_TASK_CORE 1

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

bool ESP3DGCodeHostService::processScript(const char* script,
                                          ESP3DAuthenticationLevel auth_type) {
  esp3d_log("Processing script: %s,  with authentication level=%d", script,
            static_cast<uint8_t>(auth_type));
  ESP3DScript newscript;
  newscript.type = getScriptType(script);
  newscript.id = esp3d_hal::millis();
  newscript.auth_type = auth_type;
  esp3d_log("Script type: %d", static_cast<uint8_t>(newscript.type));
  switch (newscript.type) {
    case ESP3DGcodeHostScriptType::single_command:
    case ESP3DGcodeHostScriptType::multiple_commands:
      newscript.script = script;
      _scripts.push_back(newscript);
      break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostScriptType::sd_card:
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DGcodeHostScriptType::filesystem:
      // only one FS/SD script at once is allowed
      if (getCurrentScript() != nullptr) return false;
      newscript.script = script;
      _scripts.push_back(newscript);
      return true;
      break;
    default:
      break;
  }

  return false;
}
bool ESP3DGCodeHostService::abort() { return false; }
bool ESP3DGCodeHostService::pause() { return false; }
bool ESP3DGCodeHostService::resume() { return false; }
bool ESP3DGCodeHostService::isAck(const char* cmd) { return false; }
bool ESP3DGCodeHostService::isCommand() { return false; }
bool ESP3DGCodeHostService::isAckNeeded() { return false; }
bool ESP3DGCodeHostService::startStream() { return false; }
bool ESP3DGCodeHostService::processCommand() { return false; }
bool ESP3DGCodeHostService::readNextCommand() { return false; }

ESP3DGcodeHostState ESP3DGCodeHostService::getState() {
  return ESP3DGcodeHostState::no_stream;
}

ESP3DGcodeHostError ESP3DGCodeHostService::getErrorNum() {
  return ESP3DGcodeHostError::no_error;
}

ESP3DScript* ESP3DGCodeHostService::getCurrentScript() {
  // get first not command
  for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
    if (script->type != ESP3DGcodeHostScriptType::single_command &&
        script->type != ESP3DGcodeHostScriptType::multiple_commands) {
      return &(*script);
    }
  }
  return nullptr;
}

bool ESP3DGCodeHostService::endStream() { return false; }

ESP3DGCodeHostService::ESP3DGCodeHostService() {
  _started = false;
  _xHandle = NULL;
  _current_script = NULL;
}
ESP3DGCodeHostService::~ESP3DGCodeHostService() { end(); }

void ESP3DGCodeHostService::process(ESP3DMessage* msg) {
  esp3d_log("Add message to queue");
  if (!addTxData(msg)) {
    flush();
    if (!addTxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  } else {
    flush();
  }
}

bool ESP3DGCodeHostService::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
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

bool ESP3DGCodeHostService::pushMsgToRxQueue(const uint8_t* msg, size_t size) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
      newMsgPtr->authentication_level = ESP3DAuthenticationLevel::user;
      newMsgPtr->origin = ESP3DClientType::stream;
      if (!addRxData(newMsgPtr)) {
        // delete message as cannot be added to the queue
        ESP3DClient::deleteMsg(newMsgPtr);
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

ESP3DGcodeHostScriptType ESP3DGCodeHostService::getScriptType(
    const char* script) {
  if (script[0] == '/') {
    if (strstr(script, "/sd/") == script) {
#if ESP3D_SD_CARD_FEATURE
      return ESP3DGcodeHostScriptType::sd_card;
#else
      return ESP3DGcodeHostScriptType::filesystem;
#endif  // ESP3D_SD_CARD_FEATURE
    } else {
      return ESP3DGcodeHostScriptType::filesystem;
    }
  } else {
    if (strstr(script, ";") != nullptr) {
      return ESP3DGcodeHostScriptType::multiple_commands;
    } else {
      if (strlen(script) != 0)
        return ESP3DGcodeHostScriptType::single_command;
      else
        return ESP3DGcodeHostScriptType::unknown;
    }
  }
}

void ESP3DGCodeHostService::handle() {
  if (_started) {
    // to allow to handle GCode commands when processing SD/script
    for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
      bool processing = true;
      while (processing) {
        switch (script->state) {
          case ESP3DGcodeHostState::start:
            esp3d_log("Processing script: %s", script->script.c_str());
            script->state = ESP3DGcodeHostState::end;
            break;
          case ESP3DGcodeHostState::end:
            esp3d_log("Ending script: %s", script->script.c_str());
            _scripts.erase(script++);
            processing = false;
            break;
          case ESP3DGcodeHostState::read_line:
            break;
          case ESP3DGcodeHostState::process_line:
            break;
          case ESP3DGcodeHostState::wait_for_ack:
            break;
          case ESP3DGcodeHostState::pause:
            break;
          case ESP3DGcodeHostState::paused:
            break;
          case ESP3DGcodeHostState::resume:
            break;
          case ESP3DGcodeHostState::stop:
            break;
          case ESP3DGcodeHostState::error:
            break;
          case ESP3DGcodeHostState::abort:
            break;
          case ESP3DGcodeHostState::wait:
            break;
          case ESP3DGcodeHostState::next_state:
            break;
          default:
            processing = false;
            break;
        }
        esp3d_hal::wait(10);
      }
    }

    /* if(getRxMsgsCount() > 0) {
         ESP3DMessage * msg = popRx();
         if (msg) {
             esp3dCommands.process(msg);
         }
     }
     if(getTxMsgsCount() > 0) {
         ESP3DMessage * msg = popTx();
         if (msg) {
              size_t len = uart_write_bytes(ESP3D_GCODE_HOST_PORT, msg->data,
     msg->size); if (len != msg->size) { esp3d_log_e("Error writing message %s",
     msg->data);
              }
             deleteMsg(msg);
         }
     }*/
  }
}

void ESP3DGCodeHostService::flush() {
  uint8_t loopCount = 10;
  while (loopCount && getTxMsgsCount() > 0) {
    // esp3d_log("flushing Tx messages");
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
  _current_script = NULL;
  _scripts.clear();
}
#endif  // ESP3D_GCODE_HOST_FEATURE