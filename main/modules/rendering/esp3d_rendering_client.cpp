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
#include "freertos/task.h"
#include "tasks_def.h"


ESP3DRenderingClient renderingClient;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout

// this task only collecting rendering RX data and push thenmm to Rx Queue
static void esp3d_rendering_rx_task(void *pvParameter) {
  (void)pvParameter;

  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));
    renderingClient.handle();
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
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
    return true;
  } else {
    esp3d_log_e("Rendering Task creation failed");
    _started = false;
    return false;
  }
}

void ESP3DRenderingClient::handle() {
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
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for rx failed");
    }
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }
}
