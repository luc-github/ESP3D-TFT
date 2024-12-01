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
#include "esp3d_hal.h"
#include "esp_freertos_hooks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "gcode_host/esp3d_gcode_host_service.h"
#include "serial/esp3d_serial_client.h"
#include "tasks_def.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // ESP3D_USB_SERIAL_FEATURE

#define STACKDEPTH STREAM_STACK_DEPTH
#define TASKPRIORITY STREAM_TASK_PRIORITY
#define TASKCORE STREAM_TASK_CORE

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void streamTask(void *pvParameter);

ESP3DTftStream esp3dTftstream;

/* Creates a semaphore to handle concurrent call to task stuff
 * If you wish to call *any* class function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xStreamSemaphore;

static void streamTask(void *pvParameter) {
  (void)pvParameter;
  xStreamSemaphore = xSemaphoreCreateMutex();

  if (!gcodeHostService.begin()) {
    esp3d_log_e("Failed to begin gcode host service");
  }
  esp3d_hal::wait(100);
  while (1) {
    /* Delay */

    if (pdTRUE == xSemaphoreTake(xStreamSemaphore, portMAX_DELAY)) {
      esp3dTftstream.handle();
      xSemaphoreGive(xStreamSemaphore);
    }
    esp3d_hal::wait(10);
  }

  /* A task should NEVER return */
  vTaskDelete(NULL);
}

ESP3DTftStream::ESP3DTftStream() {}

ESP3DTftStream::~ESP3DTftStream() {}

ESP3DTargetFirmware ESP3DTftStream::getTargetFirmware(bool fromSettings) {
  if (fromSettings) {
    _target_firmware = (ESP3DTargetFirmware)esp3dTftsettings.readByte(
        ESP3DSettingIndex::esp3d_target_firmware);
  }

  return _target_firmware;
}

bool ESP3DTftStream::begin() {
  // Task creation
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(streamTask, "tftStream", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Stream Task");
    // to let buffer time to empty
#if ESP3D_TFT_LOG
    esp3d_hal::wait(100);
#endif  // ESP3D_TFT_LOG
    getTargetFirmware(true);

    if (esp3dCommands.getOutputClient(true) == ESP3DClientType::serial) {
      esp3d_log("Serial client starting");
      if (serialClient.begin()) {
        esp3d_log("Serial client started");
        return true;
      } else {
        esp3d_log_e("Serial client start failed");
      }
    }
#if ESP3D_USB_SERIAL_FEATURE
    else if (esp3dCommands.getOutputClient() == ESP3DClientType::usb_serial) {
      esp3d_log("USB Serial client starting");
      if (usbSerialClient.begin()) {
        esp3d_log("USB Serial client started");
        return true;
      } else {
        esp3d_log_e("USB Serial client start failed");
      }
    }
#endif  // ESP3D_USB_SERIAL_FEATURE

  } else {
    esp3d_log_e("Stream Task creation failed");
  }
  return false;
}

void ESP3DTftStream::handle() {
  serialClient.handle();
#if ESP3D_USB_SERIAL_FEATURE
  usbSerialClient.handle();
#endif  // ESP3D_USB_SERIAL_FEATURE
}

bool ESP3DTftStream::end() {
  // TODO: need code review
  // this part is never called
  //  if called need to kill task also
  serialClient.end();
#if ESP3D_USB_SERIAL_FEATURE
  usbSerialClient.end();
#endif  // #if ESP3D_USB_SERIAL_FEATURE
  return true;
}
