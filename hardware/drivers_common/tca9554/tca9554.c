/**
 * @file tca9554.c
 * @brief TCA9554 driver.
 * @version 0.1
 * @date 2021-03-07
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "tca9554.h"

#include "esp3d_log.h"

/*********************
 *      DEFINES
 *********************/
#define BSP_EXT_IO_DEFAULT_CONFIG()                                      \
  {                                                                      \
    .ext_io0 = 1, .ext_io1 = 1, .ext_io2 = 1, .ext_io3 = 1, .lcd_bl = 0, \
    .audio_pa = 0, .boost_en = 0, .tp_rst = 0,                           \
  }

#define BSP_EXT_IO_DEFAULT_LEVEL()                                       \
  {                                                                      \
    .ext_io0 = 0, .ext_io1 = 0, .ext_io2 = 0, .ext_io3 = 0, .lcd_bl = 1, \
    .audio_pa = 1, .boost_en = 0, .tp_rst = 1,                           \
  }

#define BSP_EXT_IO_SLEEP_LEVEL()                                         \
  {                                                                      \
    .ext_io0 = 0, .ext_io1 = 0, .ext_io2 = 0, .ext_io3 = 0, .lcd_bl = 0, \
    .audio_pa = 0, .boost_en = 0, .tp_rst = 0,                           \
  }

#define BSP_EXT_IO_OUTPUT_CONFIG()                                       \
  {                                                                      \
    .ext_io0 = 0, .ext_io1 = 0, .ext_io2 = 0, .ext_io3 = 0, .lcd_bl = 0, \
    .audio_pa = 0, .boost_en = 0, .tp_rst = 0,                           \
  }

#define BSP_EXT_IO_OUTPUT_DEFAULT_LEVEL()                                \
  {                                                                      \
    .ext_io0 = 0, .ext_io1 = 0, .ext_io2 = 0, .ext_io3 = 0, .lcd_bl = 1, \
    .audio_pa = 1, .boost_en = 0, .tp_rst = 1,                           \
  }

#define TCA9554_INPUT_PORT_REG (0x00)
#define TCA9554_OUTPUT_PORT_REG (0x01)
#define TCA9554_POLARITY_INVERSION_REG (0x02)
#define TCA9554_CONFIGURATION_REG (0x03)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  union {
    struct {
      uint8_t ext_io0 : 1;
      uint8_t ext_io1 : 1;
      uint8_t ext_io2 : 1;
      uint8_t ext_io3 : 1;
      uint8_t lcd_bl : 1;
      uint8_t audio_pa : 1;
      uint8_t boost_en : 1;
      uint8_t tp_rst : 1;
    };
    uint8_t val;
  };
} ext_io_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static i2c_bus_device_handle_t _i2c_dev_tca9554 = NULL;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static esp_err_t tca9554_read_byte(uint8_t reg_addr, uint8_t *data);
static esp_err_t tca9554_write_byte(uint8_t reg_addr, uint8_t data);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the TCA9554 component.
 *
 * This function initializes the TCA9554 component by configuring the I2C bus
 * and setting the desired configuration.
 *
 * @param i2c_bus The handle to the I2C bus.
 * @param config The pointer to the TCA9554 configuration structure.
 * @return esp_err_t Returns ESP_OK if the initialization is successful,
 * otherwise returns an error code.
 */
esp_err_t tca9554_init(i2c_bus_handle_t i2c_bus,
                       const tca9554_config_t *config) {
  if (NULL != _i2c_dev_tca9554 || i2c_bus == NULL || config == NULL) {
    esp3d_log_e("tca9554 init fail due to invalid handle or config");
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t err = ESP_FAIL;
  uint8_t i2c_addr_index = 0;
  // Init the tca9554 device with the first valid address
  while (config->i2c_addr[i2c_addr_index] != 0 && err != ESP_OK) {
    esp3d_log("Checking  tca9554 i2c addr: 0x%02x",
              config->i2c_addr[i2c_addr_index]);
    _i2c_dev_tca9554 = i2c_bus_device_create(
        i2c_bus, config->i2c_addr[i2c_addr_index], config->i2c_clk_speed);
    if (_i2c_dev_tca9554 == NULL) {
      esp3d_log_e("Failed creating tca9554 device!");
      return ESP_FAIL;
    }
    err = tca9554_write_byte(TCA9554_CONFIGURATION_REG, 0xFF);
    if (err == ESP_OK) {
      esp3d_log("tca9554 device found at addr: 0x%02x",
                config->i2c_addr[i2c_addr_index]);
    } else {
      // go next address
      // i2c_bus_device_delete(_i2c_dev_tca9554);
      _i2c_dev_tca9554 = NULL;
      i2c_addr_index++;
    }
  }
  if (_i2c_dev_tca9554 == NULL) {
    esp3d_log_e("tca9554 init failed!");
    return err;
  }

  // Set the default configuration
  esp3d_log("Setup IO ext pins");
  ext_io_t io_conf = BSP_EXT_IO_DEFAULT_CONFIG();
  ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();
  if (tca9554_set_configuration(io_conf.val) != ESP_OK) {
    esp3d_log_e("tca9554 set configuration failed");
    return ESP_FAIL;
  }
  if (tca9554_write_output_pins(io_level.val) != ESP_OK) {
    esp3d_log_e("tca9554 write output pins failed");
    return ESP_FAIL;
  }

  return ESP_OK;
}

/**
 * @brief Sets the configuration of the TCA9554 device.
 *
 * This function sets the configuration of the TCA9554 device by writing the
 * specified value to the device's configuration register.
 *
 * @param val The value to be written to the configuration register.
 * @return `ESP_OK` if the configuration was successfully set, or an error code
 * if an error occurred.
 */
esp_err_t tca9554_set_configuration(uint8_t val) {
  return tca9554_write_byte(TCA9554_CONFIGURATION_REG, val);
}

/**
 * @brief Writes the output pins of the TCA9554 GPIO expander.
 *
 * This function writes the specified pin values to the output pins of the
 * TCA9554 GPIO expander.
 *
 * @param pin_val The value to be written to the output pins. Each bit in the
 * value represents the state of a pin.
 *
 * @return
 *     - ESP_OK if the operation is successful.
 *     - ESP_FAIL if an error occurred.
 */
esp_err_t tca9554_write_output_pins(uint8_t pin_val) {
  return tca9554_write_byte(TCA9554_OUTPUT_PORT_REG, pin_val);
}

/**
 * @brief Reads the output pin values of the TCA9554 GPIO expander.
 *
 * This function reads the current values of the output pins of the TCA9554 GPIO
 * expander and stores them in the provided `pin_val` variable.
 *
 * @param[in,out] pin_val Pointer to a variable where the pin values will be
 * stored.
 *
 * @return
 *     - ESP_OK if the pin values were successfully read.
 *     - ESP_FAIL if there was an error reading the pin values.
 *     - ESP_ERR_INVALID_ARG if the `pin_val` pointer is NULL.
 *     - ESP_ERR_TIMEOUT if a timeout occurred while waiting for the pin values
 * to be read.
 */
esp_err_t tca9554_read_output_pins(uint8_t *pin_val) {
  return tca9554_read_byte(TCA9554_OUTPUT_PORT_REG, pin_val);
}

/**
 * @brief Reads the input pins of the TCA9554 GPIO expander.
 *
 * This function reads the current state of the input pins of the TCA9554 GPIO
 * expander and stores the result in the provided pointer `pin_val`.
 *
 * @param[in,out] pin_val Pointer to a variable where the pin values will be
 * stored.
 *
 * @return `ESP_OK` if the operation is successful, otherwise an error code.
 */
esp_err_t tca9554_read_input_pins(uint8_t *pin_val) {
  return tca9554_read_byte(TCA9554_INPUT_PORT_REG, pin_val);
}

/**
 * @brief Sets the polarity inversion value for the TCA9554 GPIO expander.
 *
 * This function sets the polarity inversion value for the TCA9554 GPIO
 * expander. The polarity inversion value determines whether the input pins are
 * inverted or not.
 *
 * @param val The polarity inversion value to set. 0 for non-inverted, 1 for
 * inverted.
 * @return esp_err_t Returns ESP_OK if the polarity inversion value is set
 * successfully, or an error code if an error occurred.
 */
esp_err_t tca9554_set_polarity_inversion(uint8_t val) {
  return tca9554_write_byte(TCA9554_POLARITY_INVERSION_REG, val);
}

/**********************
 *   Static FUNCTIONS
 **********************/

/**
 * @brief Reads a byte from the TCA9554 register.
 *
 * This function reads a byte from the specified register address of the TCA9554
 * device.
 *
 * @param reg_addr The register address to read from.
 * @param data Pointer to store the read byte.
 * @return `ESP_OK` if the byte is successfully read, otherwise an error code.
 */
static esp_err_t tca9554_read_byte(uint8_t reg_addr, uint8_t *data) {
  if (_i2c_dev_tca9554 == NULL) {
    esp3d_log_e("tca9554 write byte fail due to invalid handle");
  }
  return i2c_bus_read_byte(_i2c_dev_tca9554, reg_addr, data);
}

/**
 * @brief Writes a byte of data to the TCA9554 register.
 *
 * This function writes a byte of data to the specified register address of the
 * TCA9554 device.
 *
 * @param reg_addr The register address to write the data to.
 * @param data The byte of data to write.
 * @return `ESP_OK` if the write operation is successful, otherwise an error
 * code.
 */
static esp_err_t tca9554_write_byte(uint8_t reg_addr, uint8_t data) {
  if (_i2c_dev_tca9554 == NULL) {
    esp3d_log_e("tca9554 write byte fail due to invalid handle");
  }
  return i2c_bus_write_byte(_i2c_dev_tca9554, reg_addr, data);
}