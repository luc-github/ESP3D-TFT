/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "tca9554.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demos.h"

static const char *TAG = "main";

void lv_tick_task(void *arg)
{
    while(1) 
    {
        vTaskDelay((10) / portTICK_PERIOD_MS);
        lv_task_handler();        
    }
}

void app_main(void)
{
    /* Initialize I2C 400KHz */
    ESP_ERROR_CHECK(bsp_i2c_init(I2C_NUM_0, 400000));
 
    /* Init TCA9554 */
    ESP_ERROR_CHECK(tca9554_init());
    ext_io_t io_conf = BSP_EXT_IO_DEFAULT_CONFIG();
    ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();
    ESP_ERROR_CHECK(tca9554_set_configuration(io_conf.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    /* LVGL init */
    lv_init();                  //内核初始化
    lv_port_disp_init();	    //接口初始化
    lv_port_indev_init();       //输入设备初始化
    // lv_port_fs_init();       //文件系统初始化
    lv_port_tick_init();

    /* example lvgl demos */
    //lv_demo_music();
    lv_demo_widgets();
    // lv_demo_keypad_encoder();
    // lv_demo_benchmark();
    // lv_demo_stress();

    xTaskCreate(lv_tick_task, "lv_tick_task", 4096, NULL, 1, NULL);
}


