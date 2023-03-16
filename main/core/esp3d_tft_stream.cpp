/*
  esp3d_tft_stream

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

#include "esp3d_tft_stream.h"

#include <string>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp_freertos_hooks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "serial/esp3d_serial_client.h"
#include "version.h"

#if ESP3D_GCODE_HOST_FEATURE
#include "gcode_host/esp3d_gcode_host_service.h"
#endif  // ESP3D_GCODE_HOST_FEATURE
#include "tasks_def.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // ESP3D_USB_SERIAL_FEATURE

#define STACKDEPTH STREAM_STACK_DEPTH
#define TASKPRIORITY 0
#define TASKCORE 1

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void streamTask(void *pvParameter);

Esp3dTftStream esp3dTftstream;

/* Creates a semaphore to handle concurrent call to task stuff
 * If you wish to call *any* class function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xStreamSemaphore;

static void streamTask(void *pvParameter) {
  (void)pvParameter;
  xStreamSemaphore = xSemaphoreCreateMutex();
#if ESP3D_GCODE_HOST_FEATURE
  if (!gcodeHostService.begin()) {
    esp3d_log_e("Failed to begin gcode host service");
  }
#endif  // ESP3D_GCODE_HOST_FEATURE
  while (1) {
    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(10));

    if (pdTRUE == xSemaphoreTake(xStreamSemaphore, portMAX_DELAY)) {
      esp3dTftstream.handle();
      xSemaphoreGive(xStreamSemaphore);
    }
  }

  /* A task should NEVER return */
  vTaskDelete(NULL);
}

Esp3dTftStream::Esp3dTftStream() {}

Esp3dTftStream::~Esp3dTftStream() {}

Esp3dTargetFirmware Esp3dTftStream::getTargetFirmware(bool fromSettings) {
  if (fromSettings) {
    _target_firmware = (Esp3dTargetFirmware)esp3dTftsettings.readByte(
        Esp3dSettingIndex::esp3d_target_firmware);
  }

  return _target_firmware;
}

bool Esp3dTftStream::begin() {
  // Task creation
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(streamTask, "tftStream", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Stream Task");
    // to let buffer time to empty
#if ESP3D_TFT_LOG
    vTaskDelay(pdMS_TO_TICKS(100));
#endif  // ESP3D_TFT_LOG
    getTargetFirmware(true);

    if (esp3dCommands.getOutputClient(true) == Esp3dClientType::serial) {
      if (serialClient.begin()) {
        return true;
      }
    }
#if ESP3D_USB_SERIAL_FEATURE
    else if (esp3dCommands.getOutputClient() == Esp3dClientType::usb_serial) {
      if (usbSerialClient.begin()) {
        return true;
      }
    }
#endif  // ESP3D_USB_SERIAL_FEATURE

  } else {
    esp3d_log_e("Stream Task creation failed");
  }
  return false;
}

void Esp3dTftStream::handle() {
  serialClient.handle();
#if ESP3D_USB_SERIAL_FEATURE
  usbSerialClient.handle();
#endif  // ESP3D_USB_SERIAL_FEATURE
}

bool Esp3dTftStream::end() {
  // TODO
  // this part is never called
  //  if called need to kill task also
  serialClient.end();
#if ESP3D_USB_SERIAL_FEATURE
  usbSerialClient.end();
#endif  // #if ESP3D_USB_SERIAL_FEATURE
  return true;
}
