/*
  esp3d-tft

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

#include "esp3d-tft.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "version.h"
#include <string>
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"
#include "lvgl.h"
#include "lvgl_helpers.h"

#define LV_TICK_PERIOD_MS 10

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_application(void);

static void lv_tick_task(void *arg)
{
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(LOG_TAG,"Clicked");
    } else if(code == LV_EVENT_VALUE_CHANGED) {
        ESP_LOGI(LOG_TAG, "Toggled");
    }
}

static void create_application(void)
{

    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
}

/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter)
{

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI bus used by the drivers */
    lvgl_driver_init();
    lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
    lv_color_t* buf2 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);


    static lv_disp_draw_buf_t draw_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.*/
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);

    static lv_disp_drv_t disp_drv;        /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = disp_driver_flush;    /*Set your driver function*/
    disp_drv.draw_buf = &draw_buf;        /*Assign the buffer to the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;   /*Set the horizontal resolution of the display*/
    disp_drv.ver_res = LV_VER_RES_MAX;   /*Set the vertical resolution of the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/



    /* Register an input device */
    static lv_indev_drv_t indev_drv;           /*Descriptor of a input device driver*/
    lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = touch_driver_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    create_application();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
    }

    /* A task should NEVER return */
    free(buf1);
    free(buf2);
    vTaskDelete(NULL);

}

Esp3DTFT::Esp3DTFT()
{

}

Esp3DTFT::~Esp3DTFT()
{

}

bool Esp3DTFT::begin()
{
    std::string target =  TFT_TARGET ;
    ESP_LOGI(LOG_TAG, "Starting ESP3D-TFT on %s ", target.c_str());
    ESP_LOGI(LOG_TAG, "Lvgl version is %s ", lv_version_info());
    //do nvs init
    ESP_LOGI(LOG_TAG, "Initialising NVS");
    esp_err_t res = nvs_flash_init();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        res = nvs_flash_init();
    }
    ESP_ERROR_CHECK(res);
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
    return true;
}

void Esp3DTFT::handle()
{

}

bool Esp3DTFT::end()
{
    return true;
}
