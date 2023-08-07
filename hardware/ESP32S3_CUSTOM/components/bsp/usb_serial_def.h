// Pins definition for  ESP32S3_CUSTOM
// Serial
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ESP3D_USB_SERIAL_BAUDRATE "115200"
#define ESP3D_USB_SERIAL_DATA_BITS (8)
#define ESP3D_USB_SERIAL_PARITY \
  (0)  // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define ESP3D_USB_SERIAL_STOP_BITS \
  (0)  // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
#define ESP3D_USB_SERIAL_RX_BUFFER_SIZE 512
#define ESP3D_USB_SERIAL_TX_BUFFER_SIZE 128
#define ESP3D_USB_SERIAL_TASK_SIZE 4096
#define ESP3D_USB_SERIAL_TASK_CORE 1
#define ESP3D_USB_SERIAL_TASK_PRIORITY 10
#define ESP3D_USB_LIB_TASK_SIZE 4096
#define ESP3D_USB_LIB_TASK_CORE 1
#define ESP3D_USB_LIB_TASK_PRIORITY 10
#ifdef __cplusplus
} /* extern "C" */
#endif