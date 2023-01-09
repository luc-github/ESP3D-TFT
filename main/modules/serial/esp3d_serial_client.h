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

#pragma once
#include <stdio.h>
#include "esp3d_client.h"
#include "esp3d_log.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

class Esp3DSerialClient : public Esp3DClient
{
public:
    Esp3DSerialClient();
    ~Esp3DSerialClient();
    bool begin();
    void handle();
    void end();
    void process(esp3d_msg_t * msg);
    bool isEndChar(uint8_t ch);
    bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
    void flush();
    bool started()
    {
        return _started;
    }
private:
    bool _started;
    pthread_mutex_t _tx_mutex;
    pthread_mutex_t _rx_mutex;
};

extern Esp3DSerialClient serialClient;

#ifdef __cplusplus
} // extern "C"
#endif