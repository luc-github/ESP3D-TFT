/*
  esp3d_tft_network

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

#include "esp3d_tft_network.h"

#include "esp3d_log.h"
#include "esp_event.h"
#include "esp_freertos_hooks.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "network/esp3d_network.h"
#include "tasks_def.h"

#define STACKDEPTH NETWORK_STACK_DEPTH
#define TASKPRIORITY 0
#define TASKCORE 0

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void networkTask(void *pvParameter);

ESP3dTFTNetwork esp3dTFTnetwork;

/* Creates a semaphore to handle concurrent call to task stuff
 * If you wish to call *any* class function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xNetworkSemaphore;

static void networkTask(void *pvParameter) {
  (void)pvParameter;
  xNetworkSemaphore = xSemaphoreCreateMutex();
#if ESP3D_WIFI_FEATURE
  esp3d_log("Init Netif and network event loop");
  ESP_ERROR_CHECK(esp_netif_init());  // desinit is not yet support so do it
                                      // once
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  vTaskDelay(pdMS_TO_TICKS(100));
#endif  // #ESP3D_WIFI_FEATURE
  esp3dNetwork.begin();
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

ESP3dTFTNetwork::ESP3dTFTNetwork() {}

ESP3dTFTNetwork::~ESP3dTFTNetwork() {}

bool ESP3dTFTNetwork::begin() {
  esp3d_log("Free mem %ld", esp_get_minimum_free_heap_size());
  // Ui creation
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(networkTask, "tftNetwork", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Network Task");
    esp3d_log("Free mem %ld", esp_get_minimum_free_heap_size());
    return true;
  } else {
    esp3d_log_e("Network Task creation failed");
    esp3d_log("Free mem %ld", esp_get_minimum_free_heap_size());
    return false;
  }
}

void ESP3dTFTNetwork::handle() {}

bool ESP3dTFTNetwork::end() { return true; }
