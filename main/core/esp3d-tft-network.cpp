/*
  esp3d-tft-network

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

#include "esp3d-tft-network.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "version.h"
#include <string>
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"




#define LV_TICK_PERIOD_MS 10
#define STACKDEPTH  4096*2
#define TASKPRIORITY 0
#define TASKCORE 1
#define LOG_TAG "tft-network"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void networkTask(void *pvParameter);


/* Creates a semaphore to handle concurrent call to task stuff
 * If you wish to call *any* class function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xNetworkSemaphore;

static void networkTask(void *pvParameter)
{

    (void) pvParameter;
    xNetworkSemaphore = xSemaphoreCreateMutex();

    while (1) {
        /* Delay */
        vTaskDelay(pdMS_TO_TICKS(10));

        if (pdTRUE == xSemaphoreTake(xNetworkSemaphore, portMAX_DELAY)) {
            esp3dTFTnetwork.handle();
            xSemaphoreGive(xNetworkSemaphore);
        }
    }

    /* A task should NEVER return */
    vTaskDelete(NULL);

}

Esp3DTFTNetwork::Esp3DTFTNetwork()
{

}

Esp3DTFTNetwork::~Esp3DTFTNetwork()
{

}

bool Esp3DTFTNetwork::begin()
{
    //Ui creation
    TaskHandle_t xHandle = NULL;
    BaseType_t  res =  xTaskCreatePinnedToCore(networkTask, "tftNetwork", STACKDEPTH, NULL, TASKPRIORITY, &xHandle, TASKCORE);
    if (res==pdPASS && xHandle) {
        ESP_LOGI (LOG_TAG, "Created Network Task");
        return true;
    } else {
        ESP_LOGE (LOG_TAG, "Network Task creation failed");
        return false;
    }

}

void Esp3DTFTNetwork::handle()
{

}

bool Esp3DTFTNetwork::end()
{
    return true;
}
