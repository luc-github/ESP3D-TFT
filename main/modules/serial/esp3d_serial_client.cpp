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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "serial_def.h"
#include "esp3d_serial_client.h"
#include "esp3d_settings.h"
#include "esp3d_log.h"
#include "esp3d_commands.h"

Esp3DSerialClient serialClient;

#define RX_FLUSH_TIME_OUT 1500 * 1000 //microseconds timeout

//this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_serial_rx_task(void *pvParameter)
{
    (void) pvParameter;
    uint8_t data[ESP3D_SERIAL_RX_BUFFER_SIZE];
    uint8_t buffer[ESP3D_SERIAL_RX_BUFFER_SIZE];
    size_t pos = 0;
    uint64_t startTimeout=0; //microseconds
    while (1) {
        /* Delay */
        vTaskDelay(pdMS_TO_TICKS(10));
        if (!serialClient.started()) {
            break;
        }
        int len = uart_read_bytes(ESP3D_SERIAL_PORT, data, (ESP3D_SERIAL_RX_BUFFER_SIZE - 1), 10 / portTICK_PERIOD_MS);
        if (len) {
            //parse data
            startTimeout = esp_timer_get_time();
            for (size_t i = 0; i < len ; i++) {
                if (pos <ESP3D_SERIAL_RX_BUFFER_SIZE) {
                    buffer[pos]= data[i];
                    pos++;
                }
                //if end of char or buffer is full
                if (serialClient.isEndChar(data[i]) || pos==ESP3D_SERIAL_RX_BUFFER_SIZE) {
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
    }
    /* A task should NEVER return */
    vTaskDelete(NULL);
}

Esp3DSerialClient::Esp3DSerialClient() {}
Esp3DSerialClient::~Esp3DSerialClient()
{
    end();
}

bool Esp3DSerialClient::isEndChar(uint8_t ch)
{
    return ((char)ch=='\n' || (char)ch=='\r');
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
    //load baudrate
    uint32_t baudrate = esp3dTFTsettings.readUint32(esp3d_baud_rate);
    if (!esp3dTFTsettings.isValidIntegerSetting(baudrate, esp3d_baud_rate)) {
        esp3d_log_w("Invalid baudrate use default");
        baudrate =esp3dTFTsettings.getDefaultIntegerSetting(esp3d_baud_rate);
    }
    esp3d_log("Use %d Serial Baud Rate", baudrate);
    uart_config_t uart_config = {
        .baud_rate = (int)baudrate,
        .data_bits = ESP3D_SERIAL_DATA_BITS,
        .parity    = ESP3D_SERIAL_PARITY,
        .stop_bits = ESP3D_SERIAL_STOP_BITS,
        .flow_ctrl = ESP3D_SERIAL_FLOW_CTRL,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = ESP3D_SERIAL_SOURCE_CLK,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif
    ESP_ERROR_CHECK(uart_driver_install(ESP3D_SERIAL_PORT, ESP3D_SERIAL_RX_BUFFER_SIZE*2, ESP3D_SERIAL_TX_BUFFER_SIZE, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ESP3D_SERIAL_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ESP3D_SERIAL_PORT, ESP3D_SERIAL_TX_PIN, ESP3D_SERIAL_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    TaskHandle_t xHandle = NULL;
    _started = true;
    BaseType_t  res =  xTaskCreatePinnedToCore(esp3d_serial_rx_task, "esp3d_serial_rx_tast", ESP3D_SERIAL_RX_TASK_SIZE, NULL, ESP3D_SERIAL_TASK_PRIORITY, &xHandle, ESP3D_SERIAL_TASK_CORE);

    if (res==pdPASS && xHandle) {
        esp3d_log ("Created Serial Task");
        esp3d_log("Serial client started");
        flush();
        return true;
    } else {
        esp3d_log_e ("Serial Task creation failed");
        _started = false;
        return false;
    }
}

bool Esp3DSerialClient::pushMsgToRxQueue(const uint8_t* msg, size_t size)
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

void Esp3DSerialClient::handle()
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
                size_t len = uart_write_bytes(ESP3D_SERIAL_PORT, msg->data, msg->size);
                if (len != msg->size) {
                    esp3d_log_e("Error writing message %s", msg->data);
                }
                deleteMsg(msg);
            }
        }
    }
}

void Esp3DSerialClient::flush()
{
    uint8_t loopCount = 10;
    while (loopCount && getTxMsgsCount() > 0) {
        //esp3d_log("flushing Tx messages");
        loopCount--;
        handle();
        uart_wait_tx_done(ESP3D_SERIAL_PORT, pdMS_TO_TICKS(500));
    }
}

void Esp3DSerialClient::end()
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
        if ( uart_is_driver_installed(ESP3D_SERIAL_PORT) && uart_driver_delete(ESP3D_SERIAL_PORT)!=ESP_OK ) {
            esp3d_log_e("Error deleting serial driver");
        }
    }
}
