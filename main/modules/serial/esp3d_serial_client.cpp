/*
  esp3d_serial_client

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
#include "esp3d_serial_client.h"

#include <stdio.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "serial_def.h"

ESP3DSerialClient serialClient;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout

void ESP3DSerialClient::readSerial() {
  static uint64_t startTimeout = 0;  // milliseconds
  int len = uart_read_bytes(ESP3D_SERIAL_PORT, _data,
                            (ESP3D_SERIAL_RX_BUFFER_SIZE - 1),
                            10 / portTICK_PERIOD_MS);
  if (len) {
    // parse data
    startTimeout = esp3d_hal::millis();
    esp3d_log("Read %d bytes", len);
    for (size_t i = 0; i < len; i++) {
      if (_bufferPos < ESP3D_SERIAL_RX_BUFFER_SIZE) {
        _buffer[_bufferPos] = _data[i];
        _bufferPos++;
      }
      // if end of char or buffer is full
      if (serialClient.isEndChar(_data[i]) ||
          _bufferPos == ESP3D_SERIAL_RX_BUFFER_SIZE) {
        // create message and push
        if (!serialClient.pushMsgToRxQueue(_buffer, _bufferPos)) {
          // send error
          esp3d_log_e("Push Message to rx queue failed");
        }
        _bufferPos = 0;
      }
    }
  }
  // if no data during a while then send them
  if (esp3d_hal::millis() - startTimeout > (RX_FLUSH_TIME_OUT) &&
      _bufferPos > 0) {
    if (!serialClient.pushMsgToRxQueue(_buffer, _bufferPos)) {
      // send error
      esp3d_log_e("Push Message to rx queue failed");
    }
    _bufferPos = 0;
  }
}

// this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_serial_rx_task(void *pvParameter) {
  (void)pvParameter;
  while (1) {
    /* Delay */
    esp3d_hal::wait(1);
    if (!serialClient.started()) {
      break;
    }
    serialClient.readSerial();
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
}

ESP3DSerialClient::ESP3DSerialClient() {
  _started = false;
  _xHandle = NULL;
  _data = NULL;
  _buffer = NULL;
  _bufferPos = 0;
}
ESP3DSerialClient::~ESP3DSerialClient() { end(); }

void ESP3DSerialClient::process(ESP3DMessage *msg) {
  esp3d_log("Add message to queue");
  if (!addTxData(msg)) {
    flush();
    if (!addTxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  } else {
    flush();
  }
}

bool ESP3DSerialClient::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}
bool ESP3DSerialClient::begin() {
  end();

  _data = (uint8_t *)malloc(ESP3D_SERIAL_RX_BUFFER_SIZE);
  if (!_data) {
    esp3d_log_e("Failed to allocate memory for buffer");
    return false;
  }

  _buffer = (uint8_t *)malloc(ESP3D_SERIAL_RX_BUFFER_SIZE);
  if (!_buffer) {
    esp3d_log_e("Failed to allocate memory for buffer");
    return false;
  }

  if (pthread_mutex_init(&_rx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for rx failed");
    return false;
  }
  setRxMutex(&_rx_mutex);

  if (pthread_mutex_init(&_tx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for tx failed");
    return false;
  }
  setTxMutex(&_tx_mutex);
  // load baudrate
  uint32_t baudrate =
      esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_baud_rate);
  if (!esp3dTftsettings.isValidIntegerSetting(
          baudrate, ESP3DSettingIndex::esp3d_baud_rate)) {
    esp3d_log_w("Invalid baudrate use default");
    baudrate = esp3dTftsettings.getDefaultIntegerSetting(
        ESP3DSettingIndex::esp3d_baud_rate);
  }
  esp3d_log("Use %ld Serial Baud Rate", baudrate);
  uart_config_t uart_config = {
      .baud_rate = (int)baudrate,
      .data_bits = ESP3D_SERIAL_DATA_BITS,
      .parity = ESP3D_SERIAL_PARITY,
      .stop_bits = ESP3D_SERIAL_STOP_BITS,
      .flow_ctrl = ESP3D_SERIAL_FLOW_CTRL,
      .rx_flow_ctrl_thresh = 0,
      .source_clk = ESP3D_SERIAL_SOURCE_CLK,
  };
  int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
  intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
  ESP_ERROR_CHECK(uart_driver_install(
      ESP3D_SERIAL_PORT, ESP3D_SERIAL_RX_BUFFER_SIZE * 2,
      ESP3D_SERIAL_TX_BUFFER_SIZE, 0, NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(ESP3D_SERIAL_PORT, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(ESP3D_SERIAL_PORT, ESP3D_SERIAL_TX_PIN,
                               ESP3D_SERIAL_RX_PIN, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  // Serial is never stopped so no need to kill the task from outside
  _started = true;
  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_serial_rx_task, "esp3d_serial_rx_task", ESP3D_SERIAL_RX_TASK_SIZE,
      NULL, ESP3D_SERIAL_TASK_PRIORITY, &_xHandle, ESP3D_SERIAL_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created Serial Task");
    esp3d_log("Serial client started");
    flush();
    return true;
  } else {
    esp3d_log_e("Serial Task creation failed");
    _started = false;
    return false;
  }
}

bool ESP3DSerialClient::pushMsgToRxQueue(const uint8_t *msg, size_t size) {
  ESP3DMessage *newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
      newMsgPtr->authentication_level = ESP3DAuthenticationLevel::admin;
#endif  // ESP3D_DISABLE_SERIAL_AUTHENTICATION
      newMsgPtr->origin = ESP3DClientType::serial;
      newMsgPtr->type = ESP3DMessageType::unique;
      if (!addRxData(newMsgPtr)) {
        // delete message as cannot be added to the queue
        ESP3DClient::deleteMsg(newMsgPtr);
        esp3d_log_e("Failed to add message to rx queue");
        return false;
      }
    } else {
      // delete message as cannot be added partially filled to the queue
      free(newMsgPtr);
      esp3d_log_e("Message creation failed");
      return false;
    }
  } else {
    esp3d_log_e("Out of memory!");
    return false;
  }
  return true;
}

void ESP3DSerialClient::handle() {
  if (_started) {
    if (getRxMsgsCount() > 0) {
      ESP3DMessage *msg = popRx();
      if (msg) {
        esp3dCommands.process(msg);
      }
    }
    if (getTxMsgsCount() > 0) {
      ESP3DMessage *msg = popTx();
      if (msg) {
        size_t len = uart_write_bytes(ESP3D_SERIAL_PORT, msg->data, msg->size);
        if (len != msg->size) {
          esp3d_log_e("Error writing message %s", msg->data);
        }
        deleteMsg(msg);
      }
    }
  }
}

void ESP3DSerialClient::flush() {
  uint8_t loopCount = 10;
  while (loopCount && getTxMsgsCount() > 0) {
    // esp3d_log("flushing Tx messages");
    loopCount--;
    handle();
    uart_wait_tx_done(ESP3D_SERIAL_PORT, pdMS_TO_TICKS(500));
  }
}

void ESP3DSerialClient::end() {
  if (_started) {
    flush();
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_log("Clearing queue Tx messages");
    clearTxQueue();
    esp3d_hal::wait(1000);
    if (pthread_mutex_destroy(&_tx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for tx failed");
    }
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for rx failed");
    }
    esp3d_log("Uninstalling Serial drivers");
    if (uart_is_driver_installed(ESP3D_SERIAL_PORT) &&
        uart_driver_delete(ESP3D_SERIAL_PORT) != ESP_OK) {
      esp3d_log_e("Error deleting serial driver");
    }
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }
  _bufferPos = 0;
  if (_data) {
    free(_data);
    _data = NULL;
  }
  if (_buffer) {
    free(_buffer);
    _buffer = NULL;
  }
}
