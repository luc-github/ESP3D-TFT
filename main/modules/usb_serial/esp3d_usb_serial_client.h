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

#pragma once
#include <pthread.h>
#include <stdio.h>

#include "esp3d_client.h"
#include "esp3d_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "usb/cdc_acm_host.h"
#include "usb/usb_host.h"
#include "usb/vcp.hpp"

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DUsbSerialClient : public ESP3DClient {
 public:
  ESP3DUsbSerialClient();
  ~ESP3DUsbSerialClient();
  bool begin();
  void handle();
  void end();
  void process(ESP3DMessage* msg);
  bool isEndChar(uint8_t ch);
  bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
  void flush();
  void connectDevice();
  void handle_rx(const uint8_t* data, size_t data_len);
  bool started() { return _started; }
  void setConnected(bool connected);
  bool isConnected() { return _connected; }
  void setBaudRate(uint32_t baudRate) { _baudrate = baudRate; }
  uint32_t getBaudRate() { return _baudrate; }

 private:
  TaskHandle_t _xHandle;
  size_t _rx_pos;
  uint8_t* _rx_buffer;
  uint32_t _baudrate;
  bool _stopConnect;
  bool _started;
  bool _connected;
  pthread_mutex_t _tx_mutex;
  pthread_mutex_t _rx_mutex;
  SemaphoreHandle_t _device_disconnected_sem;
  std::unique_ptr<CdcAcmDevice> _vcp_ptr;
};

extern ESP3DUsbSerialClient usbSerialClient;

#ifdef __cplusplus
}  // extern "C"
#endif