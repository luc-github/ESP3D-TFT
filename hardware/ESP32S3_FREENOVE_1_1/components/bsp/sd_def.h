// Pins definition for  ESP32_S3_FREENOVE
// SD SDIO
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32_S3_FREENOVE SD SDIO GPIO definition
 *
 */
#define ESP3D_SD_IS_SPI 0
#define ESP3D_SD_IS_SDIO 1

// DEFINE CMD DATA CLK PINS
// Only for reference for ESP32 devices
#define ESP3D_SDIO_CMD_PIN (38)  // GPIO 38
#define ESP3D_SDIO_CLK_PIN (39)  // GPIO 39
#define ESP3D_SDIO_D0_PIN (40)   // GPIO 40
#define ESP3D_SDIO_D1_PIN (-1)   // NC
#define ESP3D_SDIO_D2_PIN (-1)   // NC
#define ESP3D_SDIO_D3_PIN (-1)   // NC

#define ESP3D_SDIO_BIT_WIDTH 1

#define ESP3D_SD_DETECT_PIN (-1)  // GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW

//(range 400kHz - 40MHz for SDIO, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (40000)

#ifdef __cplusplus
} /* extern "C" */
#endif
