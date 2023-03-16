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

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "filesystem/esp3d_flash.h"
#include "filesystem/esp3d_sd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Esp3DGCodeHostService gcodeHostService;

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

bool Esp3DGCodeHostService::processScript(const char* script,
                                          Esp3dAuthenticationLevel auth_type) {
  esp3d_log("Processing script: %s,  with authentication level=%d", script,
            static_cast<uint8_t>(auth_type));

  return false;
}
bool Esp3DGCodeHostService::abort() { return false; }
bool Esp3DGCodeHostService::pause() { return false; }
bool Esp3DGCodeHostService::resume() { return false; }
bool Esp3DGCodeHostService::isAck(const char* cmd) { return false; }
bool Esp3DGCodeHostService::isCommand() { return false; }
bool Esp3DGCodeHostService::isAckNeeded() { return false; }
bool Esp3DGCodeHostService::startStream() { return false; }
bool Esp3DGCodeHostService::processCommand() { return false; }
bool Esp3DGCodeHostService::readNextCommand() { return false; }

Esp3dGcodeHostState Esp3DGCodeHostService::getState() {
  return Esp3dGcodeHostState::no_stream;
}

Esp3dGcodeHostError Esp3DGCodeHostService::getErrorNum() {
  return Esp3dGcodeHostError::no_error;
}

Esp3dScript* Esp3DGCodeHostService::getCurrentScript() {
  // get first not command
  for (auto script = _scripts.begin(); script != _scripts.end(); ++script) {
    if (script->type != Esp3dGcodeHostScriptType::single_command) {
      return &(*script);
    }
  }
  return nullptr;
}

bool Esp3DGCodeHostService::endStream() { return false; }

Esp3DGCodeHostService::Esp3DGCodeHostService() {
  _started = false;
  _xHandle = NULL;
  _current_script = NULL;
}
Esp3DGCodeHostService::~Esp3DGCodeHostService() { end(); }

void Esp3DGCodeHostService::process(Esp3dMessage* msg) {
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

bool Esp3DGCodeHostService::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}
bool Esp3DGCodeHostService::begin() {
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

bool Esp3DGCodeHostService::pushMsgToRxQueue(const uint8_t* msg, size_t size) {
  Esp3dMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (Esp3DClient::setDataContent(newMsgPtr, msg, size)) {
      newMsgPtr->authentication_level = Esp3dAuthenticationLevel::user;
      newMsgPtr->origin = Esp3dClientType::stream;
      if (!addRxData(newMsgPtr)) {
        // delete message as cannot be added to the queue
        Esp3DClient::deleteMsg(newMsgPtr);
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

Esp3dGcodeHostScriptType Esp3DGCodeHostService::getScriptType(
    const char* script) {
  if (script[0] == '/') {
    if (strstr(script, "/sd/") == script) {
      return Esp3dGcodeHostScriptType::sd_card;
    } else {
      return Esp3dGcodeHostScriptType::filesystem;
    }
  } else {
    if (strstr(script, ";") != nullptr) {
      return Esp3dGcodeHostScriptType::multiple_commands;
    } else {
      if (strlen(script) != 0)
        return Esp3dGcodeHostScriptType::single_command;
      else
        return Esp3dGcodeHostScriptType::unknown;
    }
  }
}

void Esp3DGCodeHostService::handle() {
  if (_started) {
    // to allow to handle GCode commands when processing SD/script
    for (auto script : _scripts) {
      bool processing = true;
      while (processing) {
        switch (script.state) {
          case Esp3dGcodeHostState::start:
            break;
          case Esp3dGcodeHostState::end:
            break;
          case Esp3dGcodeHostState::read_line:
            break;
          case Esp3dGcodeHostState::process_line:
            break;
          case Esp3dGcodeHostState::wait_for_ack:
            break;
          case Esp3dGcodeHostState::pause:
            break;
          case Esp3dGcodeHostState::paused:
            break;
          case Esp3dGcodeHostState::resume:
            break;
          case Esp3dGcodeHostState::stop:
            break;
          case Esp3dGcodeHostState::error:
            break;
          case Esp3dGcodeHostState::abort:
            break;
          case Esp3dGcodeHostState::wait:
            break;
          case Esp3dGcodeHostState::next_state:
            break;
          default:
            processing = false;
            break;
        }
      }
    }

    /* if(getRxMsgsCount() > 0) {
         Esp3dMessage * msg = popRx();
         if (msg) {
             esp3dCommands.process(msg);
         }
     }
     if(getTxMsgsCount() > 0) {
         Esp3dMessage * msg = popTx();
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

void Esp3DGCodeHostService::flush() {
  uint8_t loopCount = 10;
  while (loopCount && getTxMsgsCount() > 0) {
    // esp3d_log("flushing Tx messages");
    loopCount--;
    handle();
  }
}

void Esp3DGCodeHostService::end() {
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
