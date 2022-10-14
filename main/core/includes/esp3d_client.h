/*
  esp3d-client

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
#include <deque>
#include <pthread.h>
#include "authentication/esp3d_authentication.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NO_CLIENT,
    SERIAL_CLIENT,
    TELNET_CLIENT,
    HTTP_CLIENT,
    WEBSOCKET_CLIENT,
    ALL_CLIENTS
} esp3d_clients_t;

typedef struct {
    uint8_t * data;
    size_t size;
    esp3d_clients_t origin;
    esp3d_clients_t target;
    bool esp3d_cmd;
    esp3d_authentication_level_t authentication_level;
} esp3d_msg_t;

class Esp3DClient
{
public:
    Esp3DClient();
    ~Esp3DClient();
    virtual bool begin()
    {
        return false;
    };
    virtual void handle() {};
    virtual void end() {};
    void setTxMaxSize(size_t max)
    {
        _tx_max_size=max;
    };
    void setRxMaxSize(size_t max)
    {
        _rx_max_size=max;
    };
    bool addRXData(esp3d_msg_t * msg);
    bool addTXData(esp3d_msg_t * msg);
    esp3d_msg_t * popRx();
    esp3d_msg_t * popTx();
    void deleteMsg(esp3d_msg_t * msg);
    bool addFrontTXData(esp3d_msg_t * msg);
    void setRxMutex(pthread_mutex_t * mutex)
    {
        _rx_mutex=mutex;
    };
    void setTxMutex(pthread_mutex_t * mutex)
    {
        _tx_mutex=mutex;
    };
    bool clearRxQueue();
    bool clearTxQueue();
    size_t getRxMsgsCount()
    {
        return _rx_queue.size();
    }
    size_t getTxMsgsCount()
    {
        return _tx_queue.size();
    }
private:
    std::deque<esp3d_msg_t*> _rx_queue;
    std::deque<esp3d_msg_t*> _tx_queue;
    size_t _rx_size;
    size_t _tx_size;
    size_t _rx_max_size;
    size_t _tx_max_size;
    pthread_mutex_t *_rx_mutex;
    pthread_mutex_t *_tx_mutex;
};

#ifdef __cplusplus
} // extern "C"
#endif