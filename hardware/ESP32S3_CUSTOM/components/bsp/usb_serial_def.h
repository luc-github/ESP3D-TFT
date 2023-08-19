// Pins definition for  ESP32S3_CUSTOM
// Serial
#pragma once

#include "tasks_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP3D_USB_SERIAL_BAUDRATE "115200"
#define ESP3D_USB_SERIAL_DATA_BITS (8)
#define ESP3D_USB_SERIAL_PARITY \
  (0)  // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define ESP3D_USB_SERIAL_STOP_BITS \
  (0)  // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space

#ifdef __cplusplus
} /* extern "C" */
#endif