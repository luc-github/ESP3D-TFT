// Pins definition for ESP32_2432S028R
// SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32_2432S028R SD SPI GPIO definition
 *
 */
#define ESP3D_SD_IS_SPI 1
#define ESP3D_SD_MOSI_PIN (23)  // GPIO 23
#define ESP3D_SD_MISO_PIN (19)  // GPIO 19
#define ESP3D_SD_CLK_PIN (18)   // GPIO 18
#define ESP3D_SD_CS_PIN (5)     // GPIO 5

// #define ESP3D_SD_DETECT_PIN (-1) //GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4096)

#define SPI_ALLOCATION_SIZE (4 * 1024)
//(range 400kHz - 20MHz for SDSPI, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (20000)

// #define SD_SPI_HOST SPI2_HOST //1
#define SD_SPI_HOST SPI3_HOST  // 2

#ifdef __cplusplus
} /* extern "C" */
#endif
