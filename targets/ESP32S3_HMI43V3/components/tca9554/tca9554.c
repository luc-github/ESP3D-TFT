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

static const char *TAG= "tca9554";

static i2c_bus_device_handle_t tca9554_handle = NULL;

static esp_err_t tca9554_read_byte(uint8_t reg_addr, uint8_t *data)
{
    if (tca9554_handle==NULL) {
        ESP_LOGI(TAG, "tca9554 write byte fail due to invalid handle");
    }
    return i2c_bus_read_byte(tca9554_handle, reg_addr, data);
}

static esp_err_t tca9554_write_byte(uint8_t reg_addr, uint8_t data)
{
    if (tca9554_handle==NULL) {
        ESP_LOGI(TAG, "tca9554 write byte fail due to invalid handle");
    }
    return i2c_bus_write_byte(tca9554_handle, reg_addr, data);
}

esp_err_t tca9554_init(i2c_bus_handle_t i2c_bus_handle)
{
    if (NULL != tca9554_handle  || i2c_bus_handle==NULL) {
        ESP_LOGI(TAG, "tca9554 init fail due to invalid handle");
        return ESP_FAIL;
    }

    tca9554_handle = i2c_bus_device_create(i2c_bus_handle, TCA9554_ADDR, TCA9554_CLK_SPEED);
    if (NULL == tca9554_handle) {
        ESP_LOGE(TAG, "Failed create TCA9554 device");
        return ESP_FAIL;
    }

    esp_err_t ret = tca9554_write_byte(TCA9554_CONFIGURATION_REG, 0xFF);
    if(ESP_OK != ret) {
        ESP_LOGI(TAG, "Failed create TCA9554 with addresse %x, trying %x", TCA9554_ADDR, TCA9554A_ADDR);
        tca9554_handle = i2c_bus_device_create(i2c_bus_handle, TCA9554A_ADDR, TCA9554_CLK_SPEED);
        ret = tca9554_write_byte(TCA9554_CONFIGURATION_REG, 0xFF);
    }

    if(ESP_OK != ret) {
        ESP_LOGI(TAG, "tca9554 init fail");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Setup IO ext pins");
    ext_io_t io_conf = BSP_EXT_IO_DEFAULT_CONFIG();
    ext_io_t io_level = BSP_EXT_IO_DEFAULT_LEVEL();
    ESP_ERROR_CHECK(tca9554_set_configuration(io_conf.val));
    ESP_ERROR_CHECK(tca9554_write_output_pins(io_level.val));

    {
        ESP_LOGI(TAG, "tca9554 init ok");
        return ESP_OK;
    }
}

esp_err_t tca9554_set_configuration(uint8_t val)
{
    return tca9554_write_byte(TCA9554_CONFIGURATION_REG, val);
}

esp_err_t tca9554_write_output_pins(uint8_t pin_val)
{
    return tca9554_write_byte(TCA9554_OUTPUT_PORT_REG, pin_val);
}

esp_err_t tca9554_read_output_pins(uint8_t *pin_val)
{
    return tca9554_read_byte(TCA9554_OUTPUT_PORT_REG, pin_val);
}

esp_err_t tca9554_read_input_pins(uint8_t *pin_val)
{
    return tca9554_read_byte(TCA9554_INPUT_PORT_REG, pin_val);
}

esp_err_t tca9554_set_polarity_inversion(uint8_t val)
{
    return tca9554_write_byte(TCA9554_POLARITY_INVERSION_REG, val);
}


