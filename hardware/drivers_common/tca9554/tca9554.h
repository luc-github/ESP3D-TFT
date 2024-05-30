/**
 * @file tca9554.h
 * @brief TCA9554 driver header.
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

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "esp_err.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      TYPEDEFS
 **********************/
// Configuration structure for tca9554
typedef struct {
  uint32_t i2c_clk_speed;
  uint8_t *i2c_addr;
} tca9554_config_t;

/**
 * @brief Initializes the TCA9554 component.
 *
 * This function initializes the TCA9554 component with the provided
 * configuration.
 *
 * @param i2c_bus The handle to the I2C bus.
 * @param config Pointer to the TCA9554 configuration structure.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
esp_err_t tca9554_init(i2c_bus_handle_t i2c_bus,
                       const tca9554_config_t *config);

/**
 * @brief Sets the configuration of the TCA9554.
 *
 * This function sets the configuration of the TCA9554 by writing the specified
 * value to the device.
 *
 * @param val The value to be written to the TCA9554.
 * @return `ESP_OK` if the configuration was set successfully, otherwise an
 * error code.
 */
esp_err_t tca9554_set_configuration(uint8_t val);

/**
 * @brief Writes the output pins of the TCA9554 GPIO expander.
 *
 * This function sets the values of the output pins on the TCA9554 GPIO
 * expander.
 *
 * @param pin_val The value to be written to the output pins. Each bit in the
 * value corresponds to a pin on the expander.
 *
 * @return
 *     - ESP_OK if the operation is successful.
 *     - ESP_FAIL if an error occurred.
 *     - ESP_ERR_INVALID_ARG if the pin_val parameter is invalid.
 *     - ESP_ERR_TIMEOUT if a timeout occurred during the operation.
 */
esp_err_t tca9554_write_output_pins(uint8_t pin_val);

/**
 * @brief Reads the output pin values of the TCA9554 GPIO expander.
 *
 * This function reads the current values of the output pins on the TCA9554 GPIO
 * expander and stores the result in the provided pointer `pin_val`.
 *
 * @param[in,out] pin_val Pointer to a variable where the pin values will be
 * stored. The variable should have enough memory to store all the pin values.
 *
 * @return
 *    - ESP_OK if the pin values were successfully read.
 *    - ESP_FAIL if there was an error reading the pin values.
 *    - ESP_ERR_INVALID_ARG if `pin_val` is NULL.
 *    - ESP_ERR_TIMEOUT if the operation timed out.
 */
esp_err_t tca9554_read_output_pins(uint8_t *pin_val);

/**
 * @brief Reads the input pins of the TCA9554 GPIO expander.
 *
 * This function reads the current state of the input pins of the TCA9554 GPIO
 * expander and stores the result in the `pin_val` parameter.
 *
 * @param[in,out] pin_val Pointer to a variable where the pin values will be
 * stored. The variable should be pre-allocated by the caller.
 *
 * @return `ESP_OK` if the operation is successful, otherwise an error code.
 */
esp_err_t tca9554_read_input_pins(uint8_t *pin_val);

/**
 * @brief Sets the polarity inversion value for the TCA9554 GPIO expander.
 *
 * This function sets the polarity inversion value for the TCA9554 GPIO
 * expander. The polarity inversion determines whether the input signals are
 * inverted or not.
 *
 * @param val The polarity inversion value to set. This should be a 8-bit value.
 *            Each bit corresponds to a specific GPIO pin on the TCA9554.
 *            A value of 0 means no inversion, while a value of 1 means
 * inversion.
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_ERR_INVALID_ARG: Invalid argument
 *     - ESP_FAIL: Failed to set polarity inversion value
 */
esp_err_t tca9554_set_polarity_inversion(uint8_t val);

#ifdef __cplusplus
}
#endif
