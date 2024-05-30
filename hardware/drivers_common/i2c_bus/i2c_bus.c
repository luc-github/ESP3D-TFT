/**
 * @file i2c_bus.c
 * @brief
 * @version: modified version for ESP3D-TFT (luc lebosse 2024)
 * @date 2024-02-11
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
/*********************
 *      INCLUDES
 *********************/
#include "i2c_bus.h"

#include <stdio.h>
#include <string.h>

#include "esp3d_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/*********************
 *      DEFINES
 *********************/
#define I2C_ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define I2C_ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define I2C_BUS_FLG_DEFAULT (0)
#define I2C_BUS_MASTER_BUF_LEN (0)
#define I2C_BUS_MS_TO_WAIT portMAX_DELAY
#define I2C_BUS_TICKS_TO_WAIT (I2C_BUS_MS_TO_WAIT / portTICK_PERIOD_MS)
#define I2C_BUS_MUTEX_TICKS_TO_WAIT (I2C_BUS_MS_TO_WAIT / portTICK_PERIOD_MS)

#define I2C_BUS_CHECK(a, str, ret) \
  if (!(a)) {                      \
    esp3d_log_e("%s", str);        \
    return (ret);                  \
  }

#define I2C_BUS_CHECK_GOTO(a, str, lable) \
  if (!(a)) {                             \
    esp3d_log_e(":%s", str);              \
    goto lable;                           \
  }

#define I2C_BUS_INIT_CHECK(is_init, ret)   \
  if (!is_init) {                          \
    esp3d_log_e("i2c_bus has not inited"); \
    return (ret);                          \
  }

#define I2C_BUS_MUTEX_TAKE(mutex, ret)                           \
  if (!xSemaphoreTake(mutex, I2C_BUS_MUTEX_TICKS_TO_WAIT)) {     \
    esp3d_log_e("i2c_bus take mutex timeout, max wait = %ld ms", \
                I2C_BUS_MUTEX_TICKS_TO_WAIT);                    \
    return (ret);                                                \
  }

#define I2C_BUS_MUTEX_TAKE_MAX_DELAY(mutex, ret)                 \
  if (!xSemaphoreTake(mutex, portMAX_DELAY)) {                   \
    esp3d_log_e("i2c_bus take mutex timeout, max wait = %ld ms", \
                portMAX_DELAY);                                  \
    return (ret);                                                \
  }

#define I2C_BUS_MUTEX_GIVE(mutex, ret)        \
  if (!xSemaphoreGive(mutex)) {               \
    esp3d_log_e("i2c_bus give mutex failed"); \
    return (ret);                             \
  }

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  i2c_port_t i2c_port;      /*!<I2C port number */
  bool is_init;             /*if bus is initialized*/
  i2c_config_t conf_active; /*!<I2C active configuration */
  SemaphoreHandle_t mutex;  /* mutex to achive thread-safe*/
  int32_t ref_counter;      /*reference count*/
} i2c_bus_t;

typedef struct {
  uint8_t dev_addr;   /*device address*/
  i2c_config_t conf;  /*!<I2C active configuration */
  i2c_bus_t *i2c_bus; /*!<I2C bus*/
} i2c_bus_device_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static i2c_bus_t s_i2c_bus[I2C_NUM_MAX];

/**********************
 *  STATIC PROTOTYPES
 **********************/

static esp_err_t i2c_driver_reinit(i2c_port_t port, const i2c_config_t *conf);
static esp_err_t i2c_driver_deinit(i2c_port_t port);
static esp_err_t i2c_bus_write_reg8(i2c_bus_device_handle_t dev_handle,
                                    uint8_t mem_address, size_t data_len,
                                    const uint8_t *data);
static esp_err_t i2c_bus_read_reg8(i2c_bus_device_handle_t dev_handle,
                                   uint8_t mem_address, size_t data_len,
                                   uint8_t *data);
inline static bool i2c_config_compare(i2c_port_t port,
                                      const i2c_config_t *conf);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief Create an I2C bus instance then return a handle if created
 * successfully. Each I2C bus works in a singleton mode, which means for an i2c
 * port only one group parameter works. When i2c_bus_create is called more than
 * one time for the same i2c port, following parameter will override the
 * previous one.
 *
 * @param port I2C port number
 * @param conf Pointer to I2C bus configuration
 * @return i2c_bus_handle_t Return the I2C bus handle if created successfully,
 * return NULL if failed.
 */
i2c_bus_handle_t i2c_bus_create(i2c_port_t port, const i2c_config_t *conf) {
  I2C_BUS_CHECK(port < I2C_NUM_MAX, "I2C port error", NULL);
  I2C_BUS_CHECK(conf != NULL, "pointer = NULL error", NULL);
  I2C_BUS_CHECK(conf->mode == I2C_MODE_MASTER,
                "i2c_bus only supports master mode", NULL);

  if (s_i2c_bus[port].is_init) {
    /**if i2c_bus has been inited and configs not changed, return the handle
     * directly**/
    if (i2c_config_compare(port, conf)) {
      esp3d_log(
          "i2c%d has been inited, return handle directly, ref_counter=%ld",
          port, s_i2c_bus[port].ref_counter);
      return (i2c_bus_handle_t)&s_i2c_bus[port];
    }
  } else {
    s_i2c_bus[port].mutex = xSemaphoreCreateMutex();
    I2C_BUS_CHECK(s_i2c_bus[port].mutex != NULL,
                  "i2c_bus xSemaphoreCreateMutex failed", NULL);
    s_i2c_bus[port].ref_counter = 0;
  }

  esp_err_t ret = i2c_driver_reinit(port, conf);
  I2C_BUS_CHECK(ret == ESP_OK, "init error", NULL);
  s_i2c_bus[port].conf_active = *conf;
  s_i2c_bus[port].i2c_port = port;
  return (i2c_bus_handle_t)&s_i2c_bus[port];
}

/**
 * @brief Delete and release the I2C bus resource.
 *
 * @param p_bus_handle Point to the I2C bus handle, if delete succeed handle
 * will set to NULL.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t i2c_bus_delete(i2c_bus_handle_t *p_bus) {
  I2C_BUS_CHECK(p_bus != NULL && *p_bus != NULL, "pointer = NULL error",
                ESP_ERR_INVALID_ARG);
  i2c_bus_t *i2c_bus = (i2c_bus_t *)(*p_bus);
  I2C_BUS_INIT_CHECK(i2c_bus->is_init, ESP_FAIL);
  I2C_BUS_MUTEX_TAKE_MAX_DELAY(i2c_bus->mutex, ESP_ERR_TIMEOUT);

  /** if ref_counter == 0, de-init the bus**/
  if ((i2c_bus->ref_counter) > 0) {
    esp3d_log(
        "i2c%d is also handled by others ref_counter=%ld, won't be de-inited",
        i2c_bus->i2c_port, i2c_bus->ref_counter);
    return ESP_OK;
  }

  esp_err_t ret = i2c_driver_deinit(i2c_bus->i2c_port);
  I2C_BUS_CHECK(ret == ESP_OK, "deinit error", ret);
  vSemaphoreDelete(i2c_bus->mutex);
  *p_bus = NULL;
  return ESP_OK;
}

/**
 * @brief Scan i2c devices attached on i2c bus
 *
 * @param bus_handle I2C bus handle
 * @param buf Pointer to a buffer to save devices' address, if NULL no address
 * will be saved.
 * @param num Maximum number of addresses to save, invalid if buf set to NULL,
 * higer addresses will be discarded if num less-than the total number found on
 * the I2C bus.
 * @return uint8_t Total number of devices found on the I2C bus
 */
uint8_t i2c_bus_scan(i2c_bus_handle_t bus_handle, uint8_t *buf, uint8_t num) {
  I2C_BUS_CHECK(bus_handle != NULL, "Handle error", 0);
  i2c_bus_t *i2c_bus = (i2c_bus_t *)bus_handle;
  I2C_BUS_INIT_CHECK(i2c_bus->is_init, 0);
  uint8_t device_count = 0;
  I2C_BUS_MUTEX_TAKE_MAX_DELAY(i2c_bus->mutex, 0);
  for (uint8_t dev_address = 1; dev_address < 127; dev_address++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_address << 1) | I2C_MASTER_WRITE,
                          I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret =
        i2c_master_cmd_begin(i2c_bus->i2c_port, cmd, I2C_BUS_TICKS_TO_WAIT);

    if (ret == ESP_OK) {
      esp3d_log_e("found i2c device address = 0x%02x", dev_address);
      if (buf != NULL && device_count < num) {
        *(buf + device_count) = dev_address;
      }
      device_count++;
    }

    i2c_cmd_link_delete(cmd);
  }
  I2C_BUS_MUTEX_GIVE(i2c_bus->mutex, 0);
  return device_count;
}

/**
 * @brief Get current active clock speed.
 *
 * @param bus_handle I2C bus handle
 * @return uint32_t current clock speed
 */
uint32_t i2c_bus_get_current_clk_speed(i2c_bus_handle_t bus_handle) {
  I2C_BUS_CHECK(bus_handle != NULL, "Null Bus Handle", 0);
  i2c_bus_t *i2c_bus = (i2c_bus_t *)bus_handle;
  I2C_BUS_INIT_CHECK(i2c_bus->is_init, 0);
  return i2c_bus->conf_active.master.clk_speed;
}

/**
 * @brief Get created device number of the bus.
 *
 * @param bus_handle I2C bus handle
 * @return uint8_t created device number of the bus
 */
uint8_t i2c_bus_get_created_device_num(i2c_bus_handle_t bus_handle) {
  I2C_BUS_CHECK(bus_handle != NULL, "Null Bus Handle", 0);
  i2c_bus_t *i2c_bus = (i2c_bus_t *)bus_handle;
  I2C_BUS_INIT_CHECK(i2c_bus->is_init, 0);
  return i2c_bus->ref_counter;
}

/**
 * @brief Create an I2C device on specific bus.
 *        Dynamic configuration must be enable to achieve multiple devices with
 * different configs on a single bus. menuconfig:Bus Options->I2C Bus
 * Options->enable dynamic configuration
 *
 * @param bus_handle Point to the I2C bus handle
 * @param dev_addr i2c device address
 * @param clk_speed device specified clock frequency the i2c_bus will switch to
 * during each transfer. 0 if use current bus speed.
 * @return i2c_bus_device_handle_t return a device handle if created
 * successfully, return NULL if failed.
 */
i2c_bus_device_handle_t i2c_bus_device_create(i2c_bus_handle_t bus_handle,
                                              uint8_t dev_addr,
                                              uint32_t clk_speed) {
  I2C_BUS_CHECK(bus_handle != NULL, "Null Bus Handle", NULL);
  I2C_BUS_CHECK(clk_speed <= 400000, "clk_speed must <= 400000", NULL);
  i2c_bus_t *i2c_bus = (i2c_bus_t *)bus_handle;
  I2C_BUS_INIT_CHECK(i2c_bus->is_init, NULL);
  i2c_bus_device_t *i2c_device = calloc(1, sizeof(i2c_bus_device_t));
  I2C_BUS_CHECK(i2c_device != NULL, "calloc memory failed", NULL);
  I2C_BUS_MUTEX_TAKE_MAX_DELAY(i2c_bus->mutex, NULL);
  i2c_device->dev_addr = dev_addr;
  i2c_device->conf = i2c_bus->conf_active;

  /*if clk_speed == 0, current active clock speed will be used, else set a
   * specified value*/
  if (clk_speed != 0) {
    i2c_device->conf.master.clk_speed = clk_speed;
  }

  i2c_device->i2c_bus = i2c_bus;
  i2c_bus->ref_counter++;
  I2C_BUS_MUTEX_GIVE(i2c_bus->mutex, NULL);
  return (i2c_bus_device_handle_t)i2c_device;
}

/**
 * @brief Delete and release the I2C device resource, i2c_bus_device_delete
 * should be used in pairs with i2c_bus_device_create.
 *
 * @param p_dev_handle Point to the I2C device handle, if delete succeed handle
 * will set to NULL.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Fail
 */
esp_err_t i2c_bus_device_delete(i2c_bus_device_handle_t *p_dev_handle) {
  I2C_BUS_CHECK(p_dev_handle != NULL && *p_dev_handle != NULL,
                "Null Device Handle", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)(*p_dev_handle);
  I2C_BUS_MUTEX_TAKE_MAX_DELAY(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  i2c_device->i2c_bus->ref_counter--;
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  free(i2c_device);
  *p_dev_handle = NULL;
  return ESP_OK;
}

/**
 * @brief Get device's I2C address
 *
 * @param dev_handle I2C device handle
 * @return uint8_t I2C address, return NULL_I2C_DEV_ADDR if dev_handle is
 * invalid.
 */
uint8_t i2c_bus_device_get_address(i2c_bus_device_handle_t dev_handle) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", NULL_I2C_DEV_ADDR);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  return i2c_device->dev_addr;
}

/**
 * @brief Read multiple bytes from i2c device with 16-bit internal
 * register/memory address. If internal reg/mem address is 16-bit, please refer
 * i2c_bus_read_reg16
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to read from, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data_len Number of bytes to read
 * @param data Pointer to a buffer to save the data that was read
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_read_bytes_16(i2c_bus_device_handle_t dev_handle,
                                uint16_t mem_address, size_t data_len,
                                uint8_t *data) {
  return i2c_bus_read_reg16(dev_handle, mem_address, data_len, data);
}

/**
 * @brief Read multiple bytes from i2c device with 8-bit internal
 * register/memory address. If internal reg/mem address is 16-bit, please refer
 * i2c_bus_read_reg16
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to read from, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data_len Number of bytes to read
 * @param data Pointer to a buffer to save the data that was read
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_read_bytes(i2c_bus_device_handle_t dev_handle,
                             uint8_t mem_address, size_t data_len,
                             uint8_t *data) {
  return i2c_bus_read_reg8(dev_handle, mem_address, data_len, data);
}

/**
 * @brief Read single byte from i2c device with 8-bit internal register/memory
 * address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to read from, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data Pointer to a buffer to save the data that was read
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_read_byte(i2c_bus_device_handle_t dev_handle,
                            uint8_t mem_address, uint8_t *data) {
  return i2c_bus_read_reg8(dev_handle, mem_address, 1, data);
}

/**
 * @brief Read single bit of a byte from i2c device with 8-bit internal
 * register/memory address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to read from, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param bit_num The bit number 0 - 7 to read
 * @param data Pointer to a buffer to save the data that was read. *data == 0 ->
 * bit = 0, *data !=0 -> bit = 1.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_read_bit(i2c_bus_device_handle_t dev_handle,
                           uint8_t mem_address, uint8_t bit_num,
                           uint8_t *data) {
  uint8_t byte = 0;
  esp_err_t ret = i2c_bus_read_reg8(dev_handle, mem_address, 1, &byte);
  *data = byte & (1 << bit_num);
  *data = (*data != 0) ? 1 : 0;
  return ret;
}

/**
 * @brief Read multiple bits of a byte from i2c device with 8-bit internal
 * register/memory address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to read from, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param bit_start The bit to start from, 0 - 7, MSB at 0
 * @param length The number of bits to read, 1 - 8
 * @param data Pointer to a buffer to save the data that was read
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_read_bits(i2c_bus_device_handle_t dev_handle,
                            uint8_t mem_address, uint8_t bit_start,
                            uint8_t length, uint8_t *data) {
  uint8_t byte = 0;
  esp_err_t ret = i2c_bus_read_byte(dev_handle, mem_address, &byte);

  if (ret != ESP_OK) {
    return ret;
  }

  uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
  byte &= mask;
  byte >>= (bit_start - length + 1);
  *data = byte;
  return ret;
}

/**
 * @brief Write single byte to i2c device with 8-bit internal register/memory
 * address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data The byte to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_byte(i2c_bus_device_handle_t dev_handle,
                             uint8_t mem_address, uint8_t data) {
  return i2c_bus_write_reg8(dev_handle, mem_address, 1, &data);
}

/**
 * @brief Write single byte to i2c device with 16-bit internal register/memory
 * address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data The byte to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_byte_16(i2c_bus_device_handle_t dev_handle,
                                uint16_t mem_address, uint8_t data) {
  return i2c_bus_write_reg16(dev_handle, mem_address, 1, &data);
}

/**
 * @brief Write multiple byte to i2c device with 8-bit internal register/memory
 * address If internal reg/mem address is 16-bit, please refer
 * i2c_bus_write_reg16
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data_len Number of bytes to write
 * @param data Pointer to the bytes to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_bytes(i2c_bus_device_handle_t dev_handle,
                              uint8_t mem_address, size_t data_len,
                              const uint8_t *data) {
  return i2c_bus_write_reg8(dev_handle, mem_address, data_len, data);
}

/**
 * @brief Write multiple byte to i2c device with 16-bit internal register/memory
 * address If internal reg/mem address is 16-bit, please refer
 * i2c_bus_write_reg16
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data_len Number of bytes to write
 * @param data Pointer to the bytes to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_bytes_16(i2c_bus_device_handle_t dev_handle,
                                 uint16_t mem_address, size_t data_len,
                                 const uint8_t *data) {
  return i2c_bus_write_reg16(dev_handle, mem_address, data_len, data);
}

/**
 * @brief Write single bit of a byte to an i2c device with 8-bit internal
 * register/memory address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param bit_num The bit number 0 - 7 to write
 * @param data The bit to write, data == 0 means set bit = 0, data !=0 means set
 * bit = 1.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_bit(i2c_bus_device_handle_t dev_handle,
                            uint8_t mem_address, uint8_t bit_num,
                            uint8_t data) {
  uint8_t byte = 0;
  esp_err_t ret = i2c_bus_read_byte(dev_handle, mem_address, &byte);

  if (ret != ESP_OK) {
    return ret;
  }

  byte = (data != 0) ? (byte | (1 << bit_num)) : (byte & ~(1 << bit_num));
  return i2c_bus_write_byte(dev_handle, mem_address, byte);
}

/**
 * @brief Write multiple bits of a byte to an i2c device with 8-bit internal
 * register/memory address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param bit_start The bit to start from, 0 - 7, MSB at 0
 * @param length The number of bits to write, 1 - 8
 * @param data The bits to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_bits(i2c_bus_device_handle_t dev_handle,
                             uint8_t mem_address, uint8_t bit_start,
                             uint8_t length, uint8_t data) {
  uint8_t byte = 0;
  esp_err_t ret = i2c_bus_read_byte(dev_handle, mem_address, &byte);

  if (ret != ESP_OK) {
    return ret;
  }

  uint8_t mask = ((1 << length) - 1) << (bit_start - length + 1);
  data <<= (bit_start - length + 1);  // shift data into correct position
  data &= mask;                       // zero all non-important bits in data
  byte &= ~(mask);  // zero all important bits in existing byte
  byte |= data;     // combine data with existing byte
  return i2c_bus_write_byte(dev_handle, mem_address, byte);
}

/**
 * @brief I2C master send queued commands.
 *        This function will trigger sending all queued commands.
 *        The task will be blocked until all the commands have been sent out.
 *        If I2C_BUS_DYNAMIC_CONFIG enable, i2c_bus will dynamically check
 * configs and re-install i2c driver before each transfer, hence multiple
 * devices with different configs on a single bus can be supported.
 *        @note
 *        Only call this function in I2C master mode
 *
 * @param i2c_num I2C port number
 * @param cmd_handle I2C command handler
 * @param ticks_to_wait maximum wait ticks.
 * @param conf pointer to I2C parameter settings
 * @return esp_err_t
 */
inline static esp_err_t i2c_master_cmd_begin_with_conf(
    i2c_port_t i2c_num, i2c_cmd_handle_t cmd_handle, TickType_t ticks_to_wait,
    const i2c_config_t *conf) {
  esp_err_t ret;
#ifdef CONFIG_I2C_BUS_DYNAMIC_CONFIG
  /*if configs changed, i2c driver will reinit with new configuration*/
  if (conf != NULL && false == i2c_config_compare(i2c_num, conf)) {
    ret = i2c_driver_reinit(i2c_num, conf);
    I2C_BUS_CHECK(ret == ESP_OK, "reinit error", ret);
    s_i2c_bus[i2c_num].conf_active = *conf;
  }
#endif
  ret = i2c_master_cmd_begin(i2c_num, cmd_handle, ticks_to_wait);
  return ret;
}

/**
 * @brief I2C master send queued commands create by ``i2c_cmd_link_create`` .
 *        This function will trigger sending all queued commands.
 *        The task will be blocked until all the commands have been sent out.
 *        If I2C_BUS_DYNAMIC_CONFIG enable, i2c_bus will dynamically check
 * configs and re-install i2c driver before each transfer, hence multiple
 * devices with different configs on a single bus can be supported.
 *        @note
 *        Only call this function when ``i2c_bus_read/write_xx`` do not meet the
 * requirements
 *
 * @param dev_handle I2C device handle
 * @param cmd I2C command handler
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_cmd_begin(i2c_bus_device_handle_t dev_handle,
                            i2c_cmd_handle_t cmd) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(cmd != NULL, "I2C command error", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  I2C_BUS_INIT_CHECK(i2c_device->i2c_bus->is_init, ESP_ERR_INVALID_STATE);
  I2C_BUS_MUTEX_TAKE(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  esp_err_t ret =
      i2c_master_cmd_begin_with_conf(i2c_device->i2c_bus->i2c_port, cmd,
                                     I2C_BUS_TICKS_TO_WAIT, &i2c_device->conf);
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  return ret;
}

static esp_err_t i2c_bus_read_reg8(i2c_bus_device_handle_t dev_handle,
                                   uint8_t mem_address, size_t data_len,
                                   uint8_t *data) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(data != NULL, "data pointer error", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  I2C_BUS_INIT_CHECK(i2c_device->i2c_bus->is_init, ESP_ERR_INVALID_STATE);
  I2C_BUS_MUTEX_TAKE(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  if (mem_address != NULL_I2C_MEM_ADDR) {
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_WRITE,
                          I2C_ACK_CHECK_EN);
    i2c_master_write_byte(cmd, mem_address, I2C_ACK_CHECK_EN);
  }

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_READ,
                        I2C_ACK_CHECK_EN);
  i2c_master_read(cmd, data, data_len, I2C_MASTER_LAST_NACK);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin_with_conf(i2c_device->i2c_bus->i2c_port, cmd,
                                     I2C_BUS_TICKS_TO_WAIT, &i2c_device->conf);
  i2c_cmd_link_delete(cmd);
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  return ret;
}

esp_err_t i2c_bus_read_reg16(i2c_bus_device_handle_t dev_handle,
                             uint16_t mem_address, size_t data_len,
                             uint8_t *data) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(data != NULL, "data pointer error", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  I2C_BUS_INIT_CHECK(i2c_device->i2c_bus->is_init, ESP_ERR_INVALID_STATE);
  uint8_t memAddress8[2];
  memAddress8[0] = (uint8_t)((mem_address >> 8) & 0x00FF);
  memAddress8[1] = (uint8_t)(mem_address & 0x00FF);
  I2C_BUS_MUTEX_TAKE(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();

  if (mem_address != NULL_I2C_MEM_ADDR) {
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_WRITE,
                          I2C_ACK_CHECK_EN);
    i2c_master_write(cmd, memAddress8, 2, I2C_ACK_CHECK_EN);
  }

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_READ,
                        I2C_ACK_CHECK_EN);
  i2c_master_read(cmd, data, data_len, I2C_MASTER_LAST_NACK);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin_with_conf(i2c_device->i2c_bus->i2c_port, cmd,
                                     I2C_BUS_TICKS_TO_WAIT, &i2c_device->conf);
  i2c_cmd_link_delete(cmd);
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  return ret;
}

static esp_err_t i2c_bus_write_reg8(i2c_bus_device_handle_t dev_handle,
                                    uint8_t mem_address, size_t data_len,
                                    const uint8_t *data) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(data != NULL, "data pointer error", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  I2C_BUS_INIT_CHECK(i2c_device->i2c_bus->is_init, ESP_ERR_INVALID_STATE);
  I2C_BUS_MUTEX_TAKE(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_WRITE,
                        I2C_ACK_CHECK_EN);

  if (mem_address != NULL_I2C_MEM_ADDR) {
    i2c_master_write_byte(cmd, mem_address, I2C_ACK_CHECK_EN);
  }

  i2c_master_write(cmd, (uint8_t *)data, data_len, I2C_ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin_with_conf(i2c_device->i2c_bus->i2c_port, cmd,
                                     I2C_BUS_TICKS_TO_WAIT, &i2c_device->conf);
  i2c_cmd_link_delete(cmd);
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  return ret;
}

/**
 * @brief Write date to an i2c device with 16-bit internal reg/mem address
 *
 * @param dev_handle I2C device handle
 * @param mem_address The internal 16-bit reg/mem address to write to, set to
 * NULL_I2C_MEM_ADDR if no internal address.
 * @param data_len Number of bytes to write
 * @param data Pointer to the bytes to write.
 * @return esp_err_t
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t i2c_bus_write_reg16(i2c_bus_device_handle_t dev_handle,
                              uint16_t mem_address, size_t data_len,
                              const uint8_t *data) {
  I2C_BUS_CHECK(dev_handle != NULL, "device handle error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(data != NULL, "data pointer error", ESP_ERR_INVALID_ARG);
  i2c_bus_device_t *i2c_device = (i2c_bus_device_t *)dev_handle;
  I2C_BUS_INIT_CHECK(i2c_device->i2c_bus->is_init, ESP_ERR_INVALID_STATE);
  uint8_t memAddress8[2];
  memAddress8[0] = (uint8_t)((mem_address >> 8) & 0x00FF);
  memAddress8[1] = (uint8_t)(mem_address & 0x00FF);
  I2C_BUS_MUTEX_TAKE(i2c_device->i2c_bus->mutex, ESP_ERR_TIMEOUT);
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (i2c_device->dev_addr << 1) | I2C_MASTER_WRITE,
                        I2C_ACK_CHECK_EN);

  if (mem_address != NULL_I2C_MEM_ADDR) {
    i2c_master_write(cmd, memAddress8, 2, I2C_ACK_CHECK_EN);
  }

  i2c_master_write(cmd, (uint8_t *)data, data_len, I2C_ACK_CHECK_EN);
  i2c_master_stop(cmd);
  esp_err_t ret =
      i2c_master_cmd_begin_with_conf(i2c_device->i2c_bus->i2c_port, cmd,
                                     I2C_BUS_TICKS_TO_WAIT, &i2c_device->conf);
  i2c_cmd_link_delete(cmd);
  I2C_BUS_MUTEX_GIVE(i2c_device->i2c_bus->mutex, ESP_FAIL);
  return ret;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * @brief Reinitializes the I2C driver for the specified port with the given
 * configuration.
 *
 * This function reinitializes the I2C driver for the specified port using the
 * provided configuration.
 *
 * @param port The I2C port number.
 * @param conf Pointer to the I2C configuration structure.
 * @return `ESP_OK` if the driver is reinitialized successfully, otherwise an
 * error code.
 */
static esp_err_t i2c_driver_reinit(i2c_port_t port, const i2c_config_t *conf) {
  I2C_BUS_CHECK(port < I2C_NUM_MAX, "i2c port error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(conf != NULL, "pointer = NULL error", ESP_ERR_INVALID_ARG);

  if (s_i2c_bus[port].is_init) {
    i2c_driver_delete(port);
    s_i2c_bus[port].is_init = false;
    esp3d_log_e("i2c%d bus deinited", port);
  }

  esp_err_t ret = i2c_param_config(port, conf);
  I2C_BUS_CHECK(ret == ESP_OK, "i2c param config failed", ret);
  ret = i2c_driver_install(port, conf->mode, I2C_BUS_MASTER_BUF_LEN,
                           I2C_BUS_MASTER_BUF_LEN, I2C_BUS_FLG_DEFAULT);
  I2C_BUS_CHECK(ret == ESP_OK, "i2c driver install failed", ret);
  s_i2c_bus[port].is_init = true;
  esp3d_log("i2c%d bus inited", port);
  return ESP_OK;
}

/**
 * @brief Deinitializes the I2C driver for the specified port.
 *
 * This function deinitializes the I2C driver for the specified port.
 *
 * @param port The I2C port to deinitialize.
 * @return `ESP_OK` if the I2C driver is successfully deinitialized, or an error
 * code if an error occurred.
 */
static esp_err_t i2c_driver_deinit(i2c_port_t port) {
  I2C_BUS_CHECK(port < I2C_NUM_MAX, "i2c port error", ESP_ERR_INVALID_ARG);
  I2C_BUS_CHECK(s_i2c_bus[port].is_init == true, "i2c not inited",
                ESP_ERR_INVALID_STATE);
  i2c_driver_delete(port);  // always return ESP_OK
  s_i2c_bus[port].is_init = false;
  esp3d_log("i2c%d bus deinited", port);
  return ESP_OK;
}

/**
 * @brief compare with active i2c_bus configuration
 *
 * @param port choose which i2c_port's configuration will be compared
 * @param conf new configuration
 * @return true new configuration is equal to active configuration
 * @return false new configuration is not equal to active configuration
 */
inline static bool i2c_config_compare(i2c_port_t port,
                                      const i2c_config_t *conf) {
  if (s_i2c_bus[port].conf_active.master.clk_speed == conf->master.clk_speed &&
      s_i2c_bus[port].conf_active.sda_io_num == conf->sda_io_num &&
      s_i2c_bus[port].conf_active.scl_io_num == conf->scl_io_num &&
      s_i2c_bus[port].conf_active.scl_pullup_en == conf->scl_pullup_en &&
      s_i2c_bus[port].conf_active.sda_pullup_en == conf->sda_pullup_en) {
    return true;
  }

  return false;
}
