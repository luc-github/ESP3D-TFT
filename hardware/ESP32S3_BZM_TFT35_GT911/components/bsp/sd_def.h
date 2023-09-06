// Pins definition for ESP32 ESP32S3_BZM_TFT35_GT911
// SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32S3_BZM_TFT35_GT911 SDIO GPIO definition
 *
 */
#define ESP3D_SD_IS_SPI 0
#define ESP3D_SD_IS_SDIO 1

// DEFINE CMD DATA CLK PINS
// Only for reference for ESP32 devices
#define ESP3D_SDIO_CMD_PIN (47)  // GPIO 15
#define ESP3D_SDIO_CLK_PIN (21)  // GPIO 14
#define ESP3D_SDIO_D0_PIN (14)    // GPIO 2
#define ESP3D_SDIO_D1_PIN (13)   // NC
#define ESP3D_SDIO_D2_PIN (45)   // NC
#define ESP3D_SDIO_D3_PIN (48)   // NC
#define ESP3D_SDIO_BIT_WIDTH 4
#define SPI_ALLOCATION_SIZE (16 * 1024)

#define ESP3D_SD_DETECT_PIN (-1) //GPIO -1
 #define ESP3D_SD_DETECT_VALUE (0) //LOW
#define ESP3D_SD_FREQ (20000)
#ifdef __cplusplus
} /* extern "C" */
#endif
