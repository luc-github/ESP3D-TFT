/*
  esp3d_rendering_client

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

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DRenderingClient : public ESP3DClient {
 public:
  ESP3DRenderingClient();
  ~ESP3DRenderingClient();
  bool begin();
  void handle();
  void end();
  void process(ESP3DMessage* msg);
  bool sendGcode(const char* data);
  void flush();
  bool started() { return _started; }
  void setPolling(bool polling_on) { _polling_on = polling_on; }

 private:
  TaskHandle_t _xHandle;
  bool _started;
  bool _polling_on;
  SemaphoreHandle_t _xGuiSemaphore;
  pthread_mutex_t _rx_mutex;
};

extern ESP3DRenderingClient renderingClient;

#ifdef __cplusplus
}  // extern "C"
#endif