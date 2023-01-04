/*
  esp3d_socket_server

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
#include "esp_timer.h"
#include "esp3d_socket_server.h"
#include "esp3d_settings.h"
#include "esp3d_log.h"
#include "tasks_def.h"
#include "esp3d_commands.h"


ESP3DSocketServer socketServer;



#define RX_FLUSH_TIME_OUT 1500 * 1000 //microseconds timeout

//this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_socket_rx_task(void *pvParameter)
{
    /*   (void) pvParameter;
       uint8_t data[ESP3D_SOCKET_RX_BUFFER_SIZE];
       uint8_t buffer[ESP3D_SOCKET_RX_BUFFER_SIZE];
       size_t pos = 0;
       uint64_t startTimeout=0; //microseconds
       while (1) {
          // Delay
           vTaskDelay(pdMS_TO_TICKS(10));
           if (!serialClient.started()) {
               break;
           }
           int len = uart_read_bytes(ESP3D_SOCKET_PORT, data, (ESP3D_SOCKET_RX_BUFFER_SIZE - 1), 10 / portTICK_PERIOD_MS);
           if (len) {
               //parse data
               startTimeout = esp_timer_get_time();
               for (size_t i = 0; i < len ; i++) {
                   if (pos <ESP3D_SOCKET_RX_BUFFER_SIZE) {
                       buffer[pos]= data[i];
                       pos++;
                   }
                   //if end of char or buffer is full
                   if (serialClient.isEndChar(data[i]) || pos==ESP3D_SOCKET_RX_BUFFER_SIZE) {
                       //create message and push
                       if (!serialClient.pushMsgToRxQueue(buffer, pos)) {
                           //send error
                           esp3d_log_e("Push Message to rx queue failed");
                       }
                       pos=0;
                   }
               }
           }
           //if no data during a while then send them
           if (esp_timer_get_time()-startTimeout>(RX_FLUSH_TIME_OUT) && pos>0) {
               if (!serialClient.pushMsgToRxQueue(data, pos)) {
                   //send error
                   esp3d_log_e("Push Message to rx queue failed");
               }
               pos=0;
           }
       }*/

    socketServer.end();
}

ESP3DSocketServer::ESP3DSocketServer()
{
    _xHandle = NULL;
    _started = false;
}
ESP3DSocketServer::~ESP3DSocketServer()
{
    end();
}

bool ESP3DSocketServer::isEndChar(uint8_t ch)
{
    return ((char)ch=='\n' || (char)ch=='\r');
}

bool ESP3DSocketServer::begin()
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

    _started = true;
    BaseType_t  res =  xTaskCreatePinnedToCore(esp3d_socket_rx_task, "esp3d_socket_rx_task", ESP3D_SOCKET_TASK_SIZE, NULL, ESP3D_SOCKET_TASK_PRIORITY, &_xHandle, ESP3D_SOCKET_TASK_CORE);

    if (res==pdPASS && _xHandle) {
        esp3d_log ("Created Socket Task");
        esp3d_log("Socket Server started");
        flush();
        return true;
    } else {
        esp3d_log_e ("Serial Task creation failed");
        _started = false;
        return false;
    }
}

bool ESP3DSocketServer::pushMsgToRxQueue(const uint8_t* msg, size_t size)
{
    esp3d_msg_t * newMsgPtr = newMsg();
    if (newMsgPtr) {
        if (Esp3DClient::setDataContent (newMsgPtr,msg, size)) {
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
            newMsgPtr->authentication_level=ESP3D_LEVEL_ADMIN;
#endif // ESP3D_DISABLE_SERIAL_AUTHENTICATION 
            newMsgPtr->origin = SERIAL_CLIENT;
            if (!addRXData(newMsgPtr)) {
                //delete message as cannot be added to the queue
                Esp3DClient::deleteMsg(newMsgPtr);
                esp3d_log_e("Failed to add message to rx queue");
                return false;
            }
        } else {
            //delete message as cannot be added partially filled to the queue
            free( newMsgPtr);
            esp3d_log_e("Message creation failed");
            return false;
        }
    } else {
        esp3d_log_e("Out of memory!");
        return false;
    }
    return true;
}

void ESP3DSocketServer::handle()
{
    if (_started) {
        if(getRxMsgsCount() > 0) {
            esp3d_msg_t * msg = popRx();
            if (msg) {
                esp3dCommands.process(msg);
            }
        }
        if(getTxMsgsCount() > 0) {
            esp3d_msg_t * msg = popTx();
            if (msg) {
                //TODO:
                //send message
                //size_t len = uart_write_bytes(ESP3D_SOCKET_PORT, msg->data, msg->size);
                //if (len != msg->size) {
                //    esp3d_log_e("Error writing message %s", msg->data);
                //}
                deleteMsg(msg);
            }
        }
    }
}

void ESP3DSocketServer::flush()
{
    uint8_t loopCount = 10;
    while (loopCount && getTxMsgsCount() > 0) {
        //esp3d_log("flushing Tx messages");
        loopCount--;
        handle();
    }
}

void ESP3DSocketServer::end()
{
    if (_started) {
        flush();
        _started = false;
        esp3d_log("Clearing queue Rx messages");
        clearRxQueue();
        esp3d_log("Clearing queue Tx messages");
        clearTxQueue();
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(pthread_mutex_destroy (&_tx_mutex) != 0) {
            esp3d_log_w("Mutex destruction for tx failed");
        }
        if(pthread_mutex_destroy (&_rx_mutex) != 0) {
            esp3d_log_w("Mutex destruction for rx failed");
        }
        esp3d_log("Uninstalling Serial drivers");

    }
    if (_xHandle) {
        vTaskDelete(_xHandle);
        _xHandle = NULL;
    }
}
