/*
  esp3d-tft-stream

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

#include "esp3d-tft-stream.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "version.h"
#include <string>
#include "esp3d_log.h"
#include "serial/esp3d_serial_client.h"
#include "tasks_def.h"

#define STACKDEPTH  STREAM_STACK_DEPTH
#define TASKPRIORITY 0
#define TASKCORE 1

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void streamTask(void *pvParameter);

Esp3DTFTStream esp3dTFTstream;

/* Creates a semaphore to handle concurrent call to task stuff
 * If you wish to call *any* class function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xStreamSemaphore;

static void streamTask(void *pvParameter)
{

    (void) pvParameter;
    xStreamSemaphore = xSemaphoreCreateMutex();

    while (1) {
        /* Delay */
        vTaskDelay(pdMS_TO_TICKS(10));

        if (pdTRUE == xSemaphoreTake(xStreamSemaphore, portMAX_DELAY)) {
            esp3dTFTstream.handle();
            xSemaphoreGive(xStreamSemaphore);
        }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);

}

Esp3DTFTStream::Esp3DTFTStream()
{

}

Esp3DTFTStream::~Esp3DTFTStream()
{

}

bool Esp3DTFTStream::begin()
{
    //Task creation
    TaskHandle_t xHandle = NULL;
    BaseType_t  res =  xTaskCreatePinnedToCore(streamTask, "tftStream", STACKDEPTH, NULL, TASKPRIORITY, &xHandle, TASKCORE);
    if (res==pdPASS && xHandle) {
        esp3d_log ("Created Stream Task");
        //to let buffer time to empty
#if ESP3D_TFT_LOG
        vTaskDelay(pdMS_TO_TICKS(100));
#endif //ESP3D_TFT_LOG
//now begin serialClient
        if(serialClient.begin()) {
            return true;
        }

    } else {
        esp3d_log_e ("Stream Task creation failed");

    }
    return false;
}

void Esp3DTFTStream::handle()
{
    serialClient.handle();
}

bool Esp3DTFTStream::end()
{
    return true;
}
