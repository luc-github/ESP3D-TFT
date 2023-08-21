// Pins definition for ESP32_2432S028R
// Serial
#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"
#include "tasks_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP3D_SERIAL_PORT UART_NUM_0
#define ESP3D_SERIAL_BAUDRATE "115200"
#define ESP3D_SERIAL_RX_PIN 3
#define ESP3D_SERIAL_TX_PIN 1
#define ESP3D_SERIAL_DATA_BITS UART_DATA_8_BITS
#define ESP3D_SERIAL_PARITY UART_PARITY_DISABLE
#define ESP3D_SERIAL_STOP_BITS UART_STOP_BITS_1
#define ESP3D_SERIAL_FLOW_CTRL UART_HW_FLOWCTRL_DISABLE
#define ESP3D_SERIAL_SOURCE_CLK UART_SCLK_APB

#ifdef __cplusplus
} /* extern "C" */
#endif