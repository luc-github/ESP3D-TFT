/**
 * @file gt911.c
 * @brief gt911 Capacitive Touch Panel Controller Driver
 * @version 0.1
 * @date 2021-07-20
 * Created by luc lebosse - luc@tech-hunters - https://github.com/luc-github
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

#include "gt911.h"

#include <pthread.h>

#include "disp_def.h"
#include "driver/gpio.h"
#include "esp3d_log.h"
#include "touch_def.h"

/*********************
 *      DEFINES
 *********************/

/** @brief GT911 register map and function codes */
#define GT911_TOUCH1_XH (0x03)

#define GT911_READ_XY_REG (0x814E)
#define GT911_CONFIG_REG (0x8047)
#define GT911_PRODUCT_ID_REG (0x8140)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
  TOUCH_NOT_DETECTED = 0,
  TOUCH_DETECTED = 1,
} gt911_touch_detect_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static gt911_touch_detect_t gt911_is_touch_detected();
static esp_err_t gt911_reset(i2c_bus_device_handle_t devHandle);
static esp_err_t gt911_read_config(i2c_bus_device_handle_t devHandle);

/**********************
 *  STATIC VARIABLES
 **********************/
static i2c_bus_device_handle_t gt911_handle = NULL;
pthread_mutex_t gt911_mutex;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
esp_err_t gt911_init(i2c_bus_handle_t i2c_bus_handle) {
  if (NULL != gt911_handle || i2c_bus_handle == NULL) {
    return ESP_FAIL;
  }
  esp3d_log("GT911 init");
  gt911_handle =
      i2c_bus_device_create(i2c_bus_handle, GT911_ADDR, GT911_CLK_SPEED);

  if (NULL == gt911_handle) {
    esp3d_log_e("Failed create GT911 device");
    return ESP_FAIL;
  } else {
    esp3d_log("GT911 device created");
  }
#if GT911_RESET_PIN && GT911_RESET_PIN != -1
  esp3d_log("GT911 reset pin %d", GT911_RESET_PIN);
  gpio_config_t rst_gpio_config = {.mode = GPIO_MODE_OUTPUT,
                                   .pin_bit_mask = BIT64(GT911_RESET_PIN)};
  ESP_ERROR_CHECK(gpio_config(&rst_gpio_config));

  if (gt911_reset(gt911_handle) != ESP_OK) {
    esp3d_log_e("GT911 reset fail");
    return ESP_FAIL;
  }
#endif  // GT911_RESET_PIN

#if GT911_TOUCH_IRQ || GT911_TOUCH_IRQ_PRESS
  gpio_config_t irq_config = {
      .pin_bit_mask = BIT64(GT911_TOUCH_IRQ),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  esp_err_t result = gpio_config(&irq_config);
  assert(result == ESP_OK);
#endif
  // Read config and product id
  esp_err_t ret = gt911_read_config(gt911_handle);

  if (pthread_mutex_init(&gt911_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for GT911 failed");
    ret = ESP_FAIL;
  }

  return ret;
}

/*Will be called by the library to read the touchpad*/
void gt911_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
  static lv_coord_t last_x = 0;
  static lv_coord_t last_y = 0;
  static uint8_t dataArray[41];
  if (gt911_is_touch_detected() == TOUCH_DETECTED) {
    data->state = LV_INDEV_STATE_REL;
    esp_err_t err = i2c_bus_read_bytes_16(
        gt911_handle, GT911_READ_XY_REG + 2, 4,
        &dataArray[0]);  // only read  1 point and only read X Y
    // now can clear for next read
    i2c_bus_write_byte_16(gt911_handle, GT911_READ_XY_REG, 0);
    if (err != ESP_OK) {
      esp3d_log_e("GT911 touch read fail");
      return;
    }
    data->state = LV_INDEV_STATE_PR;
    if (gt911_mutex) {
      if (pthread_mutex_lock(&gt911_mutex) == 0) {
        last_x = ((uint16_t)dataArray[1] << 8) + dataArray[0];
        last_y = (((uint16_t)dataArray[3] << 8) + dataArray[2]);
        pthread_mutex_unlock(&gt911_mutex);
      }
    } else {
      esp3d_log_e("no mutex available");
    }

    // esp3d_log("X %d Y %d", last_x, last_y);
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

esp_err_t gt911_reset(i2c_bus_device_handle_t devHandle) {
  if (devHandle == NULL || GT911_RESET_PIN == -1 || !GT911_RESET_PIN) {
    return ESP_FAIL;
  }
  esp_err_t err = ESP_OK;
  if (gpio_set_level(GT911_RESET_PIN, 0) == ESP_OK) {
    esp3d_log("GT911 reset pin %d set to 0", GT911_RESET_PIN);
    vTaskDelay(pdMS_TO_TICKS(10));
    if (gpio_set_level(GT911_RESET_PIN, 1) == ESP_OK) {
      esp3d_log("GT911 reset pin %d set to 1", GT911_RESET_PIN);
      vTaskDelay(pdMS_TO_TICKS(10));
    } else {
      esp3d_log_e("GT911 reset pin %d set to 1 fail", GT911_RESET_PIN);
      err = ESP_FAIL;
    }
  } else {
    esp3d_log_e("GT911 reset pin %d set to 0 fail", GT911_RESET_PIN);
    err = ESP_FAIL;
  }
  return err;
}

esp_err_t gt911_read_config(i2c_bus_device_handle_t devHandle) {
  uint8_t buf[4];
  // read product id
  esp_err_t err =
      i2c_bus_read_bytes_16(devHandle, GT911_PRODUCT_ID_REG, 3, &buf[0]);
  if (err == ESP_OK) {
    esp3d_log("GT911 ID :0x%02x,0x%02x,0x%02x", buf[0], buf[1], buf[2]);
  } else {
    esp3d_log_e("GT911 product id read fail");
  }
  // read config
  esp_err_t err2 =
      i2c_bus_read_bytes_16(devHandle, GT911_CONFIG_REG, 1, &buf[3]);
  if (err2 == ESP_OK) {
    esp3d_log("GT911 config version %d", buf[3]);
  } else {
    esp3d_log_e("GT911 config read fail");
  }
  return err | err2;
}

static gt911_touch_detect_t gt911_is_touch_detected() {
  // check IRQ pin if we IRQ or IRQ and preessure
#if GT911_TOUCH_IRQ && GT911_TOUCH_IRQ_PRESS
  uint8_t irq = gpio_get_level(GT911_TOUCH_IRQ);
  if (irq == 0) {
    return TOUCH_NOT_DETECTED;
  }
#endif
  if (gt911_handle == NULL) {
    return TOUCH_NOT_DETECTED;
  }
  // check pressure if we are pressure or IRQ and pressure
#if GT911_TOUCH_PRESS || GT911_TOUCH_IRQ_PRESS
  uint8_t touch_points_num = 0;
  esp_err_t err;
  uint8_t buf[1];
  err = i2c_bus_read_bytes_16(gt911_handle, GT911_READ_XY_REG, 1, &buf[0]);
  if (err == ESP_OK) {
    if ((buf[0] & 0x80) == 0x00) {
      i2c_bus_write_byte_16(gt911_handle, GT911_READ_XY_REG, 0);
    } else {
      touch_points_num = buf[0] & 0x0f;
      if (touch_points_num == 0 || touch_points_num > 5) {
        i2c_bus_write_byte_16(gt911_handle, GT911_READ_XY_REG, 0);
      } else {
        // esp3d_log("GT911 touch detected");
        return TOUCH_DETECTED;
      }
    }

  } else {
    esp3d_log_e("GT911 touch read fail");
  }
#endif
  return TOUCH_NOT_DETECTED;
}
