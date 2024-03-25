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

#include <driver/gpio.h>

#include "esp3d_log.h"

/*********************
 *      DEFINES
 *********************/

/** @brief GT911 register map and function codes */
#define GT911_TOUCH1_XH (0x03)

#define GT911_READ_XY_REG (0x814E)
#define GT911_CONFIG_REG (0x8047)
#define GT911_PRODUCT_ID_REG (0x8140)
#define GT911_XY_MAX_REG (0x8048)

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

/**********************
 *  STATIC VARIABLES
 **********************/
static i2c_bus_device_handle_t _i2c_dev = NULL;
static const gt911_config_t *_config = NULL;
static uint16_t _gt911_x_max = 0;
static uint16_t _gt911_y_max = 0;
static volatile bool _gt911_INT = false;
static void IRAM_ATTR gt911_interrupt_handler(void *args) { _gt911_INT = true; }

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Retrieves the maximum value for the X coordinate from the GT911 touch
 * controller.
 *
 * @return The maximum value for the X coordinate.
 */
uint16_t get_gt911_x_max() {
  if (_gt911_x_max == 0) {
    esp3d_log_e("GT911 not initialized");
  }
  return _gt911_x_max;
}

/**
 * Retrieves the maximum value for the Y coordinate from the GT911 touch
 * controller.
 *
 * @return The maximum value for the Y coordinate.
 */
uint16_t get_gt911_y_max() {
  if (_gt911_y_max == 0) {
    esp3d_log_e("GT911 not initialized");
  }
  return _gt911_y_max;
}

/**
 * @brief Initializes the gt911 touch controller.
 *
 * This function initializes the gt911 touch controller with the provided
 * configuration and i2c handle.
 *
 * @param i2c_bus i2c bus handle used by the driver.
 * @param config Pointer to the configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t gt911_init(i2c_bus_handle_t i2c_bus, const gt911_config_t *config) {
  if (NULL != _i2c_dev || i2c_bus == NULL || config == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  _config = config;

  _gt911_x_max = _config->x_max;
  _gt911_y_max = _config->y_max;

  esp3d_log("GT911 init");

  // Make sure INT pin is low before reset procedure (Sets i2c address to 0x5D)
  if (GPIO_IS_VALID_OUTPUT_GPIO(config->int_pin)) {
    esp3d_log("GT911 INT pin: %d", config->int_pin);
    gpio_config_t int_gpio_cfg = {.mode = GPIO_MODE_OUTPUT,
                                  .pin_bit_mask = BIT64(config->int_pin)};
    if (gpio_config(&int_gpio_cfg) != ESP_OK) {
      esp3d_log_e("Failed to configure INT pin: %d", config->int_pin);
      return ESP_ERR_INVALID_ARG;
    }
    gpio_set_level(config->int_pin, 0);
  } else {
    if (config->int_pin != -1) {
      esp3d_log_e("Invalid INT pin: %d", config->int_pin);
      return ESP_ERR_INVALID_ARG;
    }
  }

  // Perform reset procedure
  if (GPIO_IS_VALID_OUTPUT_GPIO(config->rst_pin)) {
    esp3d_log("GT911 reset pin: %d", config->rst_pin);
    gpio_config_t rst_gpio_cfg = {.mode = GPIO_MODE_OUTPUT,
                                  .pin_bit_mask = BIT64(config->rst_pin)};
    if (gpio_config(&rst_gpio_cfg) != ESP_OK) {
      esp3d_log_e("Failed to configure RST pin: %d", config->rst_pin);
      return ESP_ERR_INVALID_ARG;
    }
    gpio_set_level(config->rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(config->rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
  } else {
    if (config->rst_pin != -1) {
      esp3d_log_e("Invalid RST pin: %d", config->rst_pin);
      return ESP_ERR_INVALID_ARG;
    }
  }

  // Reconfigure INT pin as floating input with falling edge interrupt handler
  if (GPIO_IS_VALID_GPIO(config->int_pin)) {
    gpio_config_t irq_config = {
        .pin_bit_mask = BIT64(config->int_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    esp_err_t result = gpio_config(&irq_config);
    if (result != ESP_OK) {
      esp3d_log_e("Failed to configure INT pin: %d", config->int_pin);
      return result;
    }

    gpio_install_isr_service(0);
    gpio_isr_handler_add(config->int_pin, gt911_interrupt_handler, NULL);
  } else {
    if (config->int_pin != -1) {
      esp3d_log_e("Invalid INT pin: %d", config->int_pin);
      return ESP_ERR_INVALID_ARG;
    }
  }

  uint8_t buf[4];
  esp_err_t err = ESP_FAIL;
  uint8_t i2c_addr_index = 0;

  while (config->i2c_addr[i2c_addr_index] != 0 && err != ESP_OK) {
    esp3d_log("Checking  GT911 i2c addr: 0x%02x",
              config->i2c_addr[i2c_addr_index]);
    _i2c_dev = i2c_bus_device_create(i2c_bus, config->i2c_addr[i2c_addr_index],
                                     config->i2c_clk_speed);
    if (_i2c_dev == NULL) {
      esp3d_log_e("Failed creating GT911 device!");
      return ESP_FAIL;
    }
    err = i2c_bus_read_bytes_16(_i2c_dev, GT911_PRODUCT_ID_REG, 4, &buf[0]);
    if (err == ESP_OK) {
      esp3d_log("GT911 device found at addr: 0x%02x",
                config->i2c_addr[i2c_addr_index]);
    } else {
      // go next address
      // i2c_bus_device_delete(_i2c_dev);
      _i2c_dev = NULL;
      i2c_addr_index++;
    }
  }
  if (_i2c_dev == NULL) {
    esp3d_log_e("GT911 device not found!");
    return err;
  }

  // Display Product ID
  esp3d_log("GT911 Product ID: 0x%02x,0x%02x,0x%02x,0x%02x", buf[0], buf[1],
            buf[2], buf[3]);

  // Display Config
  err = i2c_bus_read_bytes_16(_i2c_dev, GT911_CONFIG_REG, 1, &buf[0]);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to read GT911 config");
    return err;
  }
  esp3d_log("GT911 Config: 0x%02x", buf[0]);
  if (_gt911_x_max == 0 || _gt911_y_max == 0) {
    // Read X/Y Limits
    err = i2c_bus_read_bytes_16(_i2c_dev, GT911_XY_MAX_REG, 4, &buf[0]);
    if (err != ESP_OK) return err;
    if (_gt911_x_max == 0) {
      _gt911_x_max = ((uint16_t)buf[1] << 8) + buf[0];
    }
    if (_gt911_y_max == 0) {
      _gt911_y_max = ((uint16_t)buf[3] << 8) + buf[2];
    }
    if (config->swap_xy) {
      uint16_t swap_tmp = _gt911_x_max;
      _gt911_x_max = _gt911_y_max;
      _gt911_y_max = swap_tmp;
    }
  }
  esp3d_log("GT911 Limits x=%d, y=%d", _gt911_x_max, _gt911_y_max);

  return ESP_OK;
}

/**
 * Reads data from the GT911 touch controller.
 *
 * @return The GT911 data read from the touch controller.
 */
gt911_data_t gt911_read() {
  gt911_data_t data = {.is_pressed = false, .x = -1, .y = -1};
  if (_i2c_dev == NULL || _config == NULL) {
    return data;
  }
  if (gt911_is_touch_detected() == TOUCH_DETECTED) {
    uint8_t dataArray[4];
    esp_err_t err = i2c_bus_read_bytes_16(
        _i2c_dev, GT911_READ_XY_REG + 2, 4,
        &dataArray[0]);  // only read 1 point and only read X Y
    i2c_bus_write_byte_16(_i2c_dev, GT911_READ_XY_REG,
                          0);  // now can clear for next read
    if (err != ESP_OK) {
      return data;
    }

    data.is_pressed = true;
    data.x = ((uint16_t)dataArray[1] << 8) + dataArray[0];
    data.y = ((uint16_t)dataArray[3] << 8) + dataArray[2];
    if (_config->swap_xy) {
      int16_t swap_tmp = data.x;
      data.x = data.y;
      data.y = swap_tmp;
    }
    if (_config->invert_x) {
      data.x = _gt911_x_max - data.x;
    }
    if (_config->invert_y) {
      data.y = _gt911_y_max - data.y;
    }
    esp3d_log("P(%d,%d)", data.x, data.y);
  }
  return data;
}

/**********************
 *   Static FUNCTIONS
 **********************/

/**
 * @brief Checks if touch is detected by the GT911 touch controller.
 *
 * @return The touch detection status.
 */
static gt911_touch_detect_t gt911_is_touch_detected() {
  if (_i2c_dev == NULL || _config == NULL) {
    return TOUCH_NOT_DETECTED;
  }

  // Check if INT pin was triggered
  if (_config->int_pin >= 0 && _gt911_INT == false) {
    return TOUCH_NOT_DETECTED;
  }
  _gt911_INT = false;

  // Check touch pressure
  uint8_t buf[1];
  esp_err_t err =
      i2c_bus_read_bytes_16(_i2c_dev, GT911_READ_XY_REG, 1, &buf[0]);
  if (err != ESP_OK) {
    return TOUCH_NOT_DETECTED;
  }
  uint8_t touch_points_num = buf[0] & 0x0f;
  if ((buf[0] & 0x80) == 0x00 || touch_points_num == 0 ||
      touch_points_num > 5) {
    i2c_bus_write_byte_16(_i2c_dev, GT911_READ_XY_REG, 0);
  } else {
    return TOUCH_DETECTED;
  }

  return TOUCH_NOT_DETECTED;
}
