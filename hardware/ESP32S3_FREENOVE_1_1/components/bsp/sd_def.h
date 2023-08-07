// Pins definition for ESP32_ROTRICS_DEXARM35
// SD SPI
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
#define ESP3D_SDIO_CMD_PIN (15)  // GPIO 15
#define ESP3D_SDIO_CLK_PIN (14)  // GPIO 14
#define ESP3D_SDIO_D0_PIN (2)    // GPIO 2
#define ESP3D_SDIO_D1_PIN (-1)   // NC
#define ESP3D_SDIO_D2_PIN (-1)   // NC
#define ESP3D_SDIO_D3_PIN (-1)   // NC

#define SPI_ALLOCATION_SIZE (16 * 1024)

#define ESP3D_SDIO_BIT_WIDTH 1

#define ESP3D_SD_DETECT_PIN (37)  // GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW

//(range 400kHz - 40MHz for SDIO, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (40000)

#ifdef __cplusplus
} /* extern "C" */
#endif
