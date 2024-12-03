/*
  esp3d_usb_serial_client

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
#if ESP3D_USB_SERIAL_FEATURE
#include "esp3d_usb_serial_client.h"

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "usb_serial_def.h"
#include "websocket/esp3d_webui_service.h"

ESP3DUsbSerialClient usbSerialClient;

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout

/**
 * @brief Data received callback
 *
 * Just pass received data to usb serial client
 */
bool rx_callback(const uint8_t *data, size_t data_len, void *arg) {
  usbSerialClient.handle_rx(data, data_len);
  return true;
}

void ESP3DUsbSerialClient::handle_rx(const uint8_t *data, size_t data_len) {
  static uint64_t startTimeout = 0;  // microseconds
  // parse data
  startTimeout = esp3d_hal::millis();
  for (size_t i = 0; i < data_len; i++) {
    if (_rx_pos < ESP3D_USB_SERIAL_RX_BUFFER_SIZE) {
      _rx_buffer[_rx_pos] = data[i];
      _rx_pos++;
    }
    // if end of char or buffer is full
    if (isEndChar(data[i]) || _rx_pos == ESP3D_USB_SERIAL_RX_BUFFER_SIZE) {
      // create message and push
      if (!pushMsgToRxQueue(_rx_buffer, _rx_pos)) {
        // send error
        esp3d_log_e("Push Message to rx queue failed");
      }
      _rx_pos = 0;
    }
  }

  // if no data during a while then send them
  if (esp3d_hal::millis() - startTimeout > (RX_FLUSH_TIME_OUT) && _rx_pos > 0) {
    if (!pushMsgToRxQueue(_rx_buffer, _rx_pos)) {
      // send error
      esp3d_log_e("Push Message to rx queue failed");
    }
    _rx_pos = 0;
  }
}

/**
 * @brief Device event callback
 *
 * Apart from handling device disconnection it doesn't do anything useful
 */
void handle_event(const cdc_acm_host_dev_event_data_t *event, void *user_ctx) {
  switch (event->type) {
    case CDC_ACM_HOST_ERROR:
      esp3d_log_e("CDC-ACM error has occurred, err_no = %d", event->data.error);
#if ESP3D_HTTP_FEATURE
    esp3dWsWebUiService.pushNotification("USB Error occured");
#endif  // ESP3D_HTTP_FEATURE
      break;
    case CDC_ACM_HOST_DEVICE_DISCONNECTED:
      esp3d_log("Device suddenly disconnected");
      usbSerialClient.setConnected(false);
      break;
    case CDC_ACM_HOST_SERIAL_STATE:
      esp3d_log("Serial state notif 0x%04X", event->data.serial_state.val);
      break;
    case CDC_ACM_HOST_NETWORK_CONNECTION:
    default:
      break;
  }
}

void ESP3DUsbSerialClient::connectDevice() {
  if (_stopConnect || !_started || _connected || _vcp_ptr) {
    return;
  }
  const cdc_acm_host_device_config_t dev_config = {
      .connection_timeout_ms = 5000,  // 5 seconds, enough time to plug the
                                      // device in or experiment with timeout
      .out_buffer_size = ESP3D_USB_SERIAL_TX_BUFFER_SIZE,
      .in_buffer_size = ESP3D_USB_SERIAL_RX_BUFFER_SIZE,
      .event_cb = handle_event,
      .data_cb = rx_callback,
      .user_arg = NULL,
  };
  cdc_acm_line_coding_t line_coding = {
      .dwDTERate = usbSerialClient.getBaudRate(),
      .bCharFormat = ESP3D_USB_SERIAL_STOP_BITS,
      .bParityType = ESP3D_USB_SERIAL_PARITY,
      .bDataBits = ESP3D_USB_SERIAL_DATA_BITS,
  };
  // You don't need to know the device's VID and PID. Just plug in any device
  // and the VCP service will pick correct (already registered) driver for the
  // device
  esp3d_log("Waiting for USB device");
  // TODO try to identify device
  _vcp_ptr = std::unique_ptr<CdcAcmDevice>(esp_usb::VCP::open(&dev_config));

  if (_vcp_ptr == nullptr) {
    // esp3d_log_w("Failed to open VCP device, retrying...");
    return;
  }

  esp3d_hal::wait(10);

  esp3d_log("USB device found");

  if (_vcp_ptr->line_coding_set(&line_coding) == ESP_OK) {
    esp3d_log("USB Connected");
    usbSerialClient.setConnected(true);
    esp3d_hal::wait(10);
    _vcp_ptr = nullptr;
  } else {
    esp3d_log("USB device not identified");
  }
}

// this task only handle connection
static void esp3d_usb_serial_connection_task(void *pvParameter) {
  (void)pvParameter;
   esp3d_hal::wait(100);
  while (1) {
    /* Delay */
    esp3d_hal::wait(10);
    if (usbSerialClient.started()) {
      usbSerialClient.connectDevice();
    } else {
      esp3d_log("USB serial client not started");
    }
   
  }
  /* A task should NEVER return */
  vTaskDelete(NULL);
}

void ESP3DUsbSerialClient::setConnected(bool connected) {
  _connected = connected;

  if (_connected) {
    esp3d_log("USB device connected");
#if ESP3D_HTTP_FEATURE
    esp3dWsWebUiService.pushNotification("Connected");
#endif  // ESP3D_HTTP_FEATURE
    if (xSemaphoreTake(_device_disconnected_sem, portMAX_DELAY) != pdTRUE) {
      esp3d_log_e("Failed to take semaphore");
      _connected=false;
    }
  } else {
    esp3d_log("USB device disconnected");
#if ESP3D_HTTP_FEATURE
    esp3dWsWebUiService.pushNotification("Disconnected");
#endif  // ESP3D_HTTP_FEATURE
    xSemaphoreGive(_device_disconnected_sem);
    _vcp_ptr = nullptr;
  }
}

ESP3DUsbSerialClient::ESP3DUsbSerialClient() {
  _started = false;
  _connected = false;
  _device_disconnected_sem = NULL;
  _rx_buffer = NULL;
  _rx_pos = 0;
  _xHandle = NULL;
  _vcp_ptr = NULL;
  _stopConnect = false;
}
ESP3DUsbSerialClient::~ESP3DUsbSerialClient() { end(); }

void ESP3DUsbSerialClient::process(ESP3DMessage *msg) {
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

bool ESP3DUsbSerialClient::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}

bool ESP3DUsbSerialClient::begin() {
  end();
  _rx_buffer = (uint8_t *)malloc(ESP3D_USB_SERIAL_RX_BUFFER_SIZE);
  if (!_rx_buffer) {
    esp3d_log_e("Failed to allocate memory for buffer");
    return false;
  }

  _device_disconnected_sem = xSemaphoreCreateBinary();
  if (_device_disconnected_sem == NULL) {
    esp3d_log_e("Semaphore creation failed");
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
  _baudrate = esp3dTftsettings.readUint32(
      ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
  if (!esp3dTftsettings.isValidIntegerSetting(
          _baudrate, ESP3DSettingIndex::esp3d_usb_serial_baud_rate)) {
    esp3d_log_w("Invalid baudrate, %ld, use default", _baudrate);
    _baudrate = esp3dTftsettings.getDefaultIntegerSetting(
        ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
  }
  esp3d_log("Use %ld USB Serial Baud Rate", _baudrate);

  // Serial is never stopped so no need to kill the task from outside
  _started = true;
  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_usb_serial_connection_task, "esp3d_usb_serial_task",
      ESP3D_USB_SERIAL_TASK_SIZE, NULL, ESP3D_USB_SERIAL_TASK_PRIORITY,
      &_xHandle, ESP3D_USB_SERIAL_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created USB Serial Connection Task");
    esp3d_log("USB serial client started");
    flush();
    return true;
  } else {
    esp3d_log_e("USB serial Task creation failed");
    _started = false;
    return false;
  }
}

bool ESP3DUsbSerialClient::pushMsgToRxQueue(const uint8_t *msg, size_t size) {
  ESP3DMessage *newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
      newMsgPtr->authentication_level = ESP3DAuthenticationLevel::admin;
#endif  // ESP3D_DISABLE_SERIAL_AUTHENTICATION
      newMsgPtr->origin = ESP3DClientType::usb_serial;
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

void ESP3DUsbSerialClient::handle() {
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
        esp3d_log("Got message to send");
        if (_connected) {
          esp3d_log("Send message");
          // TODO: check if msg->size < ESP3D_USB_SERIAL_TX_BUFFER_SIZE
          if (_vcp_ptr && _vcp_ptr->tx_blocking(msg->data, msg->size) == ESP_OK) {
            if (!(_vcp_ptr && _vcp_ptr->set_control_line_state(true, true) == ESP_OK)) {
              esp3d_log("Failed set line");
            }
          } else {
            esp3d_log_e("Failed to send message");
          }
        }
        deleteMsg(msg);
      }
    }
  }
}

void ESP3DUsbSerialClient::flush() {}

void ESP3DUsbSerialClient::end() {
  _stopConnect = true;
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
    esp3d_log("Uninstalling USB Serial drivers");
    setConnected(false);
    vSemaphoreDelete(_device_disconnected_sem);
    _device_disconnected_sem = NULL;
  }
  if (_rx_buffer) {
    free(_rx_buffer);
    _rx_buffer = NULL;
    _rx_pos = 0;
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }
  _stopConnect = false;
}

#endif  // ESP3D_USB_SERIAL_FEATURE