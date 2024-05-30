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
#define ESP3D_SD_IS_SPI 1
#define ESP3D_SD_MOSI_PIN (9)  // GPIO 9
#define ESP3D_SD_MISO_PIN (8)  // GPIO 8
#define ESP3D_SD_CLK_PIN (7)   // GPIO 7
#define ESP3D_SD_CS_PIN (21)   // GPIO 21

#define SPI_ALLOCATION_SIZE (16 * 1024)

// #define ESP3D_SD_DETECT_PIN (-1) //GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4092)
//(range 400kHz - 20MHz for SDSPI, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (20000)

#define SD_SPI_HOST SPI2_HOST  // 1

#ifdef __cplusplus
} /* extern "C" */
#endif
