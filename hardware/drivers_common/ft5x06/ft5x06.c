/**
 * @file ft5x06.c
 * @brief ft5x06 Capacitive Touch Panel Controller Driver
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

#include "ft5x06.h"

#include <driver/gpio.h>

#include "esp3d_log.h"

/*********************
 *      DEFINES
 *********************/

/** @brief FT5x06 register map and function codes */

#define FT5x06_DEVICE_MODE (0x00)
#define FT5x06_OP_STATE (0x00)
#define FT5x06_TEST_STATE (0x40)
#define FT5x06_GESTURE_ID (0x01)
#define FT5x06_TOUCH_POINTS (0x02)

#define FT5x06_TOUCH1_EV_FLAG (0x03)
#define FT5x06_TOUCH1_XH (0x03)
#define FT5x06_TOUCH1_XL (0x04)
#define FT5x06_TOUCH1_YH (0x05)
#define FT5x06_TOUCH1_YL (0x06)

#define FT5x06_TOUCH2_EV_FLAG (0x09)
#define FT5x06_TOUCH2_XH (0x09)
#define FT5x06_TOUCH2_XL (0x0A)
#define FT5x06_TOUCH2_YH (0x0B)
#define FT5x06_TOUCH2_YL (0x0C)

#define FT5x06_TOUCH3_EV_FLAG (0x0F)
#define FT5x06_TOUCH3_XH (0x0F)
#define FT5x06_TOUCH3_XL (0x10)
#define FT5x06_TOUCH3_YH (0x11)
#define FT5x06_TOUCH3_YL (0x12)

#define FT5x06_TOUCH4_EV_FLAG (0x15)
#define FT5x06_TOUCH4_XH (0x15)
#define FT5x06_TOUCH4_XL (0x16)
#define FT5x06_TOUCH4_YH (0x17)
#define FT5x06_TOUCH4_YL (0x18)

#define FT5x06_TOUCH5_EV_FLAG (0x1B)
#define FT5x06_TOUCH5_XH (0x1B)
#define FT5x06_TOUCH5_XL (0x1C)
#define FT5x06_TOUCH5_YH (0x1D)
#define FT5x06_TOUCH5_YL (0x1E)

#define FT5x06_ID_G_THGROUP (0x80)
#define FT5x06_ID_G_THPEAK (0x81)
#define FT5x06_ID_G_THCAL (0x82)
#define FT5x06_ID_G_THWATER (0x83)
#define FT5x06_ID_G_THTEMP (0x84)
#define FT5x06_ID_G_THDIFF (0x85)
#define FT5x06_ID_G_CTRL (0x86)
#define FT5x06_ID_G_TIME_ENTER_MONITOR (0x87)
#define FT5x06_ID_G_PERIODACTIVE (0x88)
#define FT5x06_ID_G_PERIODMONITOR (0x89)
#define FT5x06_ID_G_AUTO_CLB_MODE (0xA0)
#define FT5x06_ID_G_LIB_VERSION_H (0xA1)
#define FT5x06_ID_G_LIB_VERSION_L (0xA2)
#define FT5x06_ID_G_CIPHER (0xA3)
#define FT5x06_ID_G_MODE (0xA4)
#define FT5x06_ID_G_PMODE (0xA5)
#define FT5x06_ID_G_FIRMID (0xA6)
#define FT5x06_ID_G_STATE (0xA7)
#define FT5x06_ID_G_FT5201ID (0xA8)
#define FT5x06_ID_G_ERR (0xA9)

#define FT5x06_MIN_VERSION (0xB2)
#define FT5x06_SUB_VERSION (0xB3)

#define FT5x06_READ_H_X_MAX (0x0C)
#define FT5x06_READ_L_X_MAX (0x0D)
#define FT5x06_READ_H_Y_MAX (0x0E)
#define FT5x06_READ_L_Y_MAX (0x0F)

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
static i2c_bus_device_handle_t _i2c_dev = NULL;
static const ft5x06_config_t *_config = NULL;
static uint16_t _ft5x06_x_max = 0;
static uint16_t _ft5x06_y_max = 0;
static volatile bool _ft5x06_INT = false;
static void IRAM_ATTR ft5x06_interrupt_handler(void *args) {
  _ft5x06_INT = true;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Retrieves the maximum value for the X coordinate from the ft5x06 touch
 * controller.
 *
 * @return The maximum value for the X coordinate.
 */
uint16_t get_ft5x06_x_max() {
  if (_ft5x06_x_max == 0) {
    esp3d_log_e("ft5x06 not initialized");
  }
  return _ft5x06_x_max;
}

/**
 * Retrieves the maximum value for the Y coordinate from the ft5x06 touch
 * controller.
 *
 * @return The maximum value for the Y coordinate.
 */
uint16_t get_ft5x06_y_max() {
  if (_ft5x06_y_max == 0) {
    esp3d_log_e("ft5x06 not initialized");
  }
  return _ft5x06_y_max;
}

/**
 * @brief Initializes the ft5x06 touch controller.
 *
 * This function initializes the ft5x06 touch controller with the provided
 * configuration and i2c handle.
 *
 * @param i2c_bus i2c bus handle used by the driver.
 * @param config Pointer to the configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t ft5x06_init(i2c_bus_handle_t i2c_bus, const ft5x06_config_t *config) {
  if (NULL != _i2c_dev || i2c_bus == NULL || config == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  _config = config;

  _ft5x06_x_max = _config->x_max;
  _ft5x06_y_max = _config->y_max;

  esp3d_log("ft5x06 init, x_max=%d, y_max=%d", _ft5x06_x_max, _ft5x06_y_max);

  // Make sure INT pin is low before reset procedure
  if (GPIO_IS_VALID_OUTPUT_GPIO(config->int_pin)) {
    esp3d_log("ft5x06 INT pin: %d", config->int_pin);
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
    esp3d_log("ft5x06 reset pin: %d", config->rst_pin);
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
    gpio_isr_handler_add(config->int_pin, ft5x06_interrupt_handler, NULL);
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
    esp3d_log("Checking  ft5x06 i2c addr: 0x%02x",
              config->i2c_addr[i2c_addr_index]);
    _i2c_dev = i2c_bus_device_create(i2c_bus, config->i2c_addr[i2c_addr_index],
                                     config->i2c_clk_speed);
    if (_i2c_dev == NULL) {
      esp3d_log_e("Failed creating ft5x06 device!");
      return ESP_FAIL;
    }
    // Valid touching detect threshold
    err = i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THGROUP, 70);

    // valid touching peak detect threshold
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THPEAK, 60);

    // Touch focus threshold
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THCAL, 16);

    // threshold when there is surface water
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THWATER, 60);

    // threshold of temperature compensation
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THTEMP, 10);

    // Touch difference threshold
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_THDIFF, 20);

    // Delay to enter 'Monitor' status (s)
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_TIME_ENTER_MONITOR, 2);

    // Period of 'Active' status (ms)
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_PERIODACTIVE, 12);

    // Timer to enter 'idle' when in 'Monitor' (ms)
    err |= i2c_bus_write_byte(_i2c_dev, FT5x06_ID_G_PERIODMONITOR, 40);

    // Get Vendor ID
    err |= i2c_bus_read_byte(_i2c_dev, FT5x06_ID_G_FT5201ID, &buf[0]);

    // Get Firmware ID
    err |= i2c_bus_read_byte(_i2c_dev, FT5x06_ID_G_FIRMID, &buf[1]);

    // Get FW Minor version
    err |= i2c_bus_read_byte(_i2c_dev, FT5x06_MIN_VERSION, &buf[2]);

    // Get FW Sub version
    err |= i2c_bus_read_byte(_i2c_dev, FT5x06_SUB_VERSION, &buf[3]);

    if (err == ESP_OK) {
      esp3d_log("ft5x06 device found at addr: 0x%02x",
                config->i2c_addr[i2c_addr_index]);
    } else {
      // go next address
      // i2c_bus_device_delete(_i2c_dev);
      _i2c_dev = NULL;
      i2c_addr_index++;
    }
  }
  if (_i2c_dev == NULL) {
    esp3d_log_e("ft5x06 device not found!");
    return err;
  }

  // Display Product ID
  esp3d_log("ft5x06 Vendor ID: 0x%02x, Version: %d.%d.%d", buf[0], buf[1],
            buf[2], buf[3]);

  // Get Width and Height resolution of the touch panel if not set in config
  /// Note: even following specs the values are not consistents
  if (_ft5x06_x_max == 0 || _ft5x06_y_max == 0) {
    err = i2c_bus_write_byte(_i2c_dev, FT5x06_DEVICE_MODE, FT5x06_TEST_STATE);
    if (err == ESP_OK) {
      // Read X/Y Width
      err = i2c_bus_read_bytes(_i2c_dev, FT5x06_READ_H_X_MAX, 4, buf);

      if (_ft5x06_x_max == 0) {
        _ft5x06_x_max = (((uint16_t)buf[0] & 0x0F) << 8) + buf[1];
      }
      if (_ft5x06_y_max == 0) {
        _ft5x06_y_max = (((uint16_t)buf[2] & 0x0F) << 8) + buf[3];
      }

      err = i2c_bus_write_byte(_i2c_dev, FT5x06_DEVICE_MODE, FT5x06_OP_STATE);
    }
    if (err != ESP_OK) {
      esp3d_log_e("Failed to read ft5x06 config");
      return err;
    }
  }
  if (config->swap_xy) {
    uint16_t swap_tmp = _ft5x06_x_max;
    _ft5x06_x_max = _ft5x06_y_max;
    _ft5x06_y_max = swap_tmp;
  }
  esp3d_log("ft5x06 Limits x=%d, y=%d", _ft5x06_x_max, _ft5x06_y_max);
  if (_ft5x06_x_max == 0 || _ft5x06_y_max == 0) {
    esp3d_log_e("ft5x06 invalid limits");
    return ESP_FAIL;
  }
  return ESP_OK;
}

/**
 * Reads data from the ft5x06 touch controller.
 *
 * @return The ft5x06 data read from the touch controller.
 */
ft5x06_data_t ft5x06_read() {
  ft5x06_data_t data = {.is_pressed = false, .x = -1, .y = -1};
  if (_i2c_dev == NULL || _config == NULL) {
    return data;
  }
  if (ft5x06_is_touch_detected() == TOUCH_DETECTED) {
    uint8_t dataArray[4];
    esp_err_t err =
        i2c_bus_read_bytes(_i2c_dev, FT5x06_TOUCH1_XH, 4, dataArray);
    if (err != ESP_OK) {
      return data;
    }

    data.is_pressed = true;
    data.x = ((uint16_t)(dataArray[0] & 0x0f) << 8) + dataArray[1];
    data.y = ((uint16_t)(dataArray[2] & 0x0f) << 8) + dataArray[3];
    if (_config->swap_xy) {
      int16_t swap_tmp = data.x;
      data.x = data.y;
      data.y = _ft5x06_y_max - swap_tmp;
    }

    if (_config->invert_x) {
      data.x = _ft5x06_x_max - data.x;
    }
    if (_config->invert_y) {
      data.y = _ft5x06_y_max - data.y;
    }
    esp3d_log("P(%d,%d)", data.x, data.y);
  }
  return data;
}

/**********************
 *   Static FUNCTIONS
 **********************/

/**
 * @brief Checks if touch is detected by the ft5x06 touch controller.
 *
 * @return The touch detection status.
 */
static ft5x06_touch_detect_t ft5x06_is_touch_detected() {
  // check IRQ pin if we IRQ or IRQ and preessure
#if WITH_FT5X06_INT
  uint8_t irq = gpio_get_level(_config->int_pin);

  if (irq == 0) {
    return TOUCH_NOT_DETECTED;
  }
#endif  // WITH_FT5X06_INT

  uint8_t touch_points_num = 0;
  esp_err_t err =
      i2c_bus_read_byte(_i2c_dev, FT5x06_TOUCH_POINTS, &touch_points_num);
  if (err == ESP_OK) {
    if (!(touch_points_num == 0 || touch_points_num == 255)) {
      return TOUCH_DETECTED;
    } else {
      return TOUCH_NOT_DETECTED;
    }
  }

  return TOUCH_NOT_DETECTED;
}
