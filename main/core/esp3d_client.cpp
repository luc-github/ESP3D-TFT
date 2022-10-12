/*
  esp3d-client class base object

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

#include "esp3d_client.h"
#include <string.h>
#include <string>

Esp3DClient::Esp3DClient()
{
    _started = false;
    _rx_size=0;
    _tx_size=0;
    _rx_max_size=512;
    _tx_max_size=512;
    _rx_mutex =nullptr;
    _tx_mutex=nullptr;
}
bool Esp3DClient::clearRxQueue()
{
    while (!_rx_queue.empty()) {
        deleteMsg(popRx());
    }
    _rx_size=0;
    return true;
}

esp3d_msg_t * Esp3DClient::popRx()
{
    esp3d_msg_t* msg = _rx_queue.front();
    _rx_size-=msg->size;
    _rx_queue.pop_front();
    return msg;
}
esp3d_msg_t * Esp3DClient::popTx()
{
    esp3d_msg_t* msg = _tx_queue.front();
    _tx_size-=msg->size;
    _tx_queue.pop_front();
    return msg;
}

void Esp3DClient::deleteMsg(esp3d_msg_t * msg)
{
    delete msg->data;
    delete msg;
}

bool Esp3DClient::clearTxQueue()
{
    while (!_tx_queue.empty()) {
        deleteMsg(popTx());
    }
    //sanity check just in case
    _tx_size=0;
    return true;
}
Esp3DClient::~Esp3DClient()
{
    _started = false;
    clearTxQueue();
    clearRxQueue();
}

bool Esp3DClient::addRXData(esp3d_msg_t * msg)
{
    bool res = false;
    if (_rx_mutex) {
        if(pthread_mutex_lock(_rx_mutex) == 0) {
            if (msg->size + _rx_size <= _rx_max_size) {
                _rx_queue.push_back (msg);
                _rx_size+=msg->size;
                res = true;
            }
            pthread_mutex_unlock(_rx_mutex);
        }
    }
    return res;
}
bool Esp3DClient::addTXData(esp3d_msg_t * msg)
{
    bool res = false;
    if (_tx_mutex) {
        if(pthread_mutex_lock(_tx_mutex) == 0) {
            if (msg->size + _tx_size <= _tx_max_size) {
                _tx_queue.push_back (msg);
                _tx_size+=msg->size;
                res = true;
            }
            pthread_mutex_unlock(_tx_mutex);
        }
    }
    return res;
}
bool Esp3DClient::addFrontTXData(esp3d_msg_t * msg)
{
    bool res = false;
    if (_tx_mutex) {
        if(pthread_mutex_lock(_tx_mutex) == 0) {
            if (msg->size + _tx_size <= _tx_max_size) {
                _tx_queue.push_front (msg);
                _tx_size+=msg->size;
                res = true;
            }
            pthread_mutex_unlock(_tx_mutex);
        }
    }
    return res;
}

