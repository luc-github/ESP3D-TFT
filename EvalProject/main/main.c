/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "main";

extern void status_option(void);

void app_main(void)
{
    while (1) {
        ESP_LOGI(TAG, "Hello World");
        status_option();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
