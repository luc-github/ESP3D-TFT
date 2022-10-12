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
#include <stdio.h>
#include "esp3d_serial_client.h"
#include "esp3d_log.h"

Esp3DSerialClient serialClient;

Esp3DSerialClient::Esp3DSerialClient() {}
Esp3DSerialClient::~Esp3DSerialClient()
{
    end();
}
bool Esp3DSerialClient::begin()
{
    end();
    if(pthread_mutex_init (&_rx_mutex, NULL) != 0) {
        esp3d_log_e("Mutex creation for rx failed");
        return false;
    }
    setRxMutex(&_rx_mutex);

    if(pthread_mutex_init (&_tx_mutex, NULL) != 0) {
        esp3d_log_e("Mutex creation for tx failed");
        return false;
    }
    setTxMutex(&_tx_mutex);
    esp3d_log("Serial client started");
    return true;
}
void Esp3DSerialClient::handle() {}
void Esp3DSerialClient::end()
{
    if(pthread_mutex_destroy (&_tx_mutex) == 0) {
        esp3d_log_w("Mutex destruction for tx failed");
    }
    if(pthread_mutex_destroy (&_rx_mutex) == 0) {
        esp3d_log_w("Mutex destruction for rx failed");
    }
}
