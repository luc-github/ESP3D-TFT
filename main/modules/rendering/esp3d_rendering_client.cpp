/*
  esp3d_rendering_client

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
#include "esp3d_rendering_client.h"

#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_gcode_parser_service.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "freertos/task.h"
#include "gcode_host/esp3d_gcode_host_service.h"
#include "tasks_def.h"

ESP3DRenderingClient renderingClient;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout

#define ESP3D_POLLING_INTERVAL 3000  // milliseconds

// this task only collecting rendering RX data and push thenmm to Rx Queue
static void esp3d_rendering_rx_task(void *pvParameter) {
  (void)pvParameter;

  while (1) {
    /* Delay */
    esp3d_hal::wait(10);
    renderingClient.handle();
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
}

// this function is send manually by user interface so no need a queue
bool ESP3DRenderingClient::sendGcode(const char *data) {
  if (data == nullptr || strlen(data) == 0) {
    return false;
  }
  // the command must end with \n, so we add it if not present
  std::string cmd = "";
  ESP3DRequest requestId = {.id = 0};
  if (data[strlen(data) - 1] != '\n') {
    cmd = data;
    cmd += "\n";
  } else {
    cmd = data;
  }
  return esp3dCommands.dispatch(
      cmd.c_str(), ESP3DClientType::stream, requestId, ESP3DMessageType::unique,
      ESP3DClientType::rendering, ESP3DAuthenticationLevel::admin);
}

ESP3DRenderingClient::ESP3DRenderingClient() {
  _started = false;
  _xHandle = NULL;
}
ESP3DRenderingClient::~ESP3DRenderingClient() { end(); }

void ESP3DRenderingClient::process(ESP3DMessage *msg) {
  // esp3d_log("Rendering client received message: %s", (char *)msg->data);
  if (!addRxData(msg)) {
    esp3d_log_e("Push Message to rx queue failed");
    flush();
  }
}

bool ESP3DRenderingClient::begin() {
  end();

  if (pthread_mutex_init(&_rx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for rx failed");
    return false;
  }
  setRxMutex(&_rx_mutex);

  _xGuiSemaphore = xSemaphoreCreateMutex();

  _started = true;
  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_rendering_rx_task, "esp3d_rendering_rx_task",
      ESP3D_RENDERING_RX_TASK_SIZE, NULL, ESP3D_RENDERING_TASK_PRIORITY,
      &_xHandle, ESP3D_RENDERING_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created Rendering Task");
    esp3d_log("Rendering client started");
    flush();
    if (esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_polling_on) == 1) {
      setPolling(true);
    } else {
      setPolling(false);
    }
    return true;
  } else {
    esp3d_log_e("Rendering Task creation failed");
    _started = false;
    return false;
  }
}

void ESP3DRenderingClient::handle() {
  static uint64_t now = esp3d_hal::millis();
  static uint8_t polling_cmd_index = 0;
  if (_started) {
    if (getRxMsgsCount() > 0) {
      if (pdTRUE == xSemaphoreTake(_xGuiSemaphore, portMAX_DELAY)) {
        ESP3DMessage *msg = popRx();
        if (msg) {
          esp3d_log("Rendering client received message: %s", (char *)msg->data);
          esp3dGcodeParser.processCommand((char *)msg->data);
          deleteMsg(msg);
        };
        xSemaphoreGive(_xGuiSemaphore);
      }
    }
    if (_polling_on) {
      if (esp3d_hal::millis() - now >
          ESP3D_POLLING_INTERVAL / ESP3D_POLLING_COMMANDS_COUNT) {
        esp3d_log("Polling interval reached, list size is %d",
                  gcodeHostService.getScriptsListSize());
        const char **pollingCommands = esp3dGcodeParser.getPollingCommands();
        if (pollingCommands == nullptr) {
          return;
        }
        // reset index if needed
        if (polling_cmd_index >= ESP3D_POLLING_COMMANDS_COUNT)
          polling_cmd_index = 0;
        // send command if not already in queue and last response is too old
        bool is_in_queue = gcodeHostService.hasStreamListCommand(
            pollingCommands[polling_cmd_index]);
        bool is_recently_processed =
            ((esp3d_hal::millis() -
              esp3dGcodeParser.getPollingCommandsLastRun(polling_cmd_index)) <
             ESP3D_POLLING_INTERVAL);
        esp3d_log("Command %s in queue: %d, recently processed: %d",
                  pollingCommands[polling_cmd_index], is_in_queue,
                  is_recently_processed);
        if (!is_in_queue && !is_recently_processed) {
          esp3d_log("Sending command %s", pollingCommands[polling_cmd_index]);
          sendGcode(pollingCommands[polling_cmd_index]);
        } else {
          esp3d_log_w("Command %s already in queue or recently processed",
                      pollingCommands[polling_cmd_index]);
        }
        polling_cmd_index++;
        now = esp3d_hal::millis();
      }
    }
  }
}

void ESP3DRenderingClient::flush() {
  uint8_t loopCount = 10;
  while (loopCount && getRxMsgsCount() > 0) {
    // esp3d_log("flushing Rx messages");
    loopCount--;
    handle();
  }
}

void ESP3DRenderingClient::end() {
  if (_started) {
    flush();
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_hal::wait(1000);
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for rx failed");
    }
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }
}
