/**
 * @file ft5x06.c
 * @brief FT5x06 Capacitive Touch Panel Controller Driver
 * @version 0.1
 * @date 2021-01-13
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0

 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "driver/gpio.h"
#include "esp3d_log.h"
#include "ft5x06.h"
#include "touch_def.h"
#include "disp_def.h"

/*********************
 *      DEFINES
 *********************/

/** @brief FT5x06 register map and function codes */

#define FT5x06_DEVICE_MODE      (0x00)
#define FT5x06_GESTURE_ID       (0x01)
#define FT5x06_TOUCH_POINTS     (0x02)

#define FT5x06_TOUCH1_EV_FLAG   (0x03)
#define FT5x06_TOUCH1_XH        (0x03)
#define FT5x06_TOUCH1_XL        (0x04)
#define FT5x06_TOUCH1_YH        (0x05)
#define FT5x06_TOUCH1_YL        (0x06)

#define FT5x06_TOUCH2_EV_FLAG   (0x09)
#define FT5x06_TOUCH2_XH        (0x09)
#define FT5x06_TOUCH2_XL        (0x0A)
#define FT5x06_TOUCH2_YH        (0x0B)
#define FT5x06_TOUCH2_YL        (0x0C)

#define FT5x06_TOUCH3_EV_FLAG   (0x0F)
#define FT5x06_TOUCH3_XH        (0x0F)
#define FT5x06_TOUCH3_XL        (0x10)
#define FT5x06_TOUCH3_YH        (0x11)
#define FT5x06_TOUCH3_YL        (0x12)

#define FT5x06_TOUCH4_EV_FLAG   (0x15)
#define FT5x06_TOUCH4_XH        (0x15)
#define FT5x06_TOUCH4_XL        (0x16)
#define FT5x06_TOUCH4_YH        (0x17)
#define FT5x06_TOUCH4_YL        (0x18)

#define FT5x06_TOUCH5_EV_FLAG   (0x1B)
#define FT5x06_TOUCH5_XH        (0x1B)
#define FT5x06_TOUCH5_XL        (0x1C)
#define FT5x06_TOUCH5_YH        (0x1D)
#define FT5x06_TOUCH5_YL        (0x1E)

#define FT5x06_ID_G_THGROUP             (0x80)
#define FT5x06_ID_G_THPEAK              (0x81)
#define FT5x06_ID_G_THCAL               (0x82)
#define FT5x06_ID_G_THWATER             (0x83)
#define FT5x06_ID_G_THTEMP              (0x84)
#define FT5x06_ID_G_THDIFF              (0x85)
#define FT5x06_ID_G_CTRL                (0x86)
#define FT5x06_ID_G_TIME_ENTER_MONITOR  (0x87)
#define FT5x06_ID_G_PERIODACTIVE        (0x88)
#define FT5x06_ID_G_PERIODMONITOR       (0x89)
#define FT5x06_ID_G_AUTO_CLB_MODE       (0xA0)
#define FT5x06_ID_G_LIB_VERSION_H       (0xA1)
#define FT5x06_ID_G_LIB_VERSION_L       (0xA2)
#define FT5x06_ID_G_CIPHER              (0xA3)
#define FT5x06_ID_G_MODE                (0xA4)
#define FT5x06_ID_G_PMODE               (0xA5)
#define FT5x06_ID_G_FIRMID              (0xA6)
#define FT5x06_ID_G_STATE               (0xA7)
#define FT5x06_ID_G_FT5201ID            (0xA8)
#define FT5x06_ID_G_ERR                 (0xA9)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    TOUCH_NOT_DETECTED = 0,
    TOUCH_DETECTED = 1,
} ft5x06_touch_detect_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static ft5x06_touch_detect_t ft5x06_is_touch_detected();

/**********************
 *  STATIC VARIABLES
 **********************/
static i2c_bus_device_handle_t ft5x06_handle = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
esp_err_t ft5x06_init(i2c_bus_handle_t i2c_bus_handle)
{
    if (NULL != ft5x06_handle || i2c_bus_handle == NULL) {
        return ESP_FAIL;
    }

    ft5x06_handle = i2c_bus_device_create(i2c_bus_handle, FT5x06_ADDR, FT5x06_CLK_SPEED);

    if (NULL == ft5x06_handle) {
        esp3d_log_e("Failed create FX5X06 device");
        return ESP_FAIL;
    }

#if FT5x06_TOUCH_IRQ || FT5x06_TOUCH_IRQ_PRESS
    gpio_config_t irq_config = {
        .pin_bit_mask = BIT64(FT5x06_TOUCH_IRQ),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t result = gpio_config(&irq_config);
    assert(result == ESP_OK);
#endif

    // Valid touching detect threshold
    esp_err_t ret = i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THGROUP, 70);

    // valid touching peak detect threshold
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THPEAK, 60);

    // Touch focus threshold
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THCAL, 16);

    // threshold when there is surface water
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THWATER, 60);

    // threshold of temperature compensation
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THTEMP, 10);

    // Touch difference threshold
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_THDIFF, 20);

    // Delay to enter 'Monitor' status (s)
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_TIME_ENTER_MONITOR, 2);

    // Period of 'Active' status (ms)
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_PERIODACTIVE, 12);

    // Timer to enter 'idle' when in 'Monitor' (ms)
    ret |= i2c_bus_write_byte(ft5x06_handle, FT5x06_ID_G_PERIODMONITOR, 40);

    if(ret == ESP_OK) {
        esp3d_log( "ft5x06 init ok");
    } else {
        esp3d_log_e("ft5x06 init fail");
    }
    return ret;
}

/*Will be called by the library to read the touchpad*/
void ft5x06_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    static uint8_t dataArray[4];

    if(ft5x06_is_touch_detected() == TOUCH_DETECTED) {
        data->state = LV_INDEV_STATE_PR;
        i2c_bus_read_bytes(ft5x06_handle,FT5x06_TOUCH1_XH, 4, dataArray);
#if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
        last_y = DISP_VER_RES_MAX - ((dataArray[0] & 0x0f) << 8) - dataArray[1];
        last_x = ((dataArray[2] & 0x0f) << 8) + dataArray[3];
#else  // portrait mode
        last_x = ((dataArray[0] & 0x0f) << 8) + dataArray[1];
        last_y = ((dataArray[2] & 0x0f) << 8) + dataArray[3];
#endif

        //esp3d_log("X %d y %d", last_x, last_y);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}


/**********************
 *   Static FUNCTIONS
 **********************/
static ft5x06_touch_detect_t ft5x06_is_touch_detected()
{
    // check IRQ pin if we IRQ or IRQ and preessure
#if FT5x06_TOUCH_IRQ && FT5x06_TOUCH_IRQ_PRESS
    uint8_t irq = gpio_get_level(FT5x06_TOUCH_IRQ);

    if (irq == 0) {

        return TOUCH_NOT_DETECTED;
    }
#endif
    // check pressure if we are pressure or IRQ and pressure
#if  FT5x06_TOUCH_PRESS || FT5x06_TOUCH_IRQ_PRESS
    uint8_t touch_points_num = 0;
    i2c_bus_read_byte(ft5x06_handle,FT5x06_TOUCH_POINTS, &touch_points_num);
    if (!(touch_points_num==0 || touch_points_num==255)) {
        return TOUCH_DETECTED;
    } else {
        return TOUCH_NOT_DETECTED;
    }
#endif
    return TOUCH_NOT_DETECTED;
}
