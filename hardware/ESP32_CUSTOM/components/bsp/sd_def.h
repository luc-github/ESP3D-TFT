// Pins definition for ESP32_CUSTOM
// SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32_2432S028R SD SPI GPIO definition
 *
 */
#define ESP3D_SD_IS_SPI 1       // TTGO T1 LOLIN D32 PRO
#define ESP3D_SD_MOSI_PIN (23)  // GPIO 23 (15)
#define ESP3D_SD_MISO_PIN (19)  // GPIO 19 (2)
#define ESP3D_SD_CLK_PIN (18)   // GPIO 18 (14)
#define ESP3D_SD_CS_PIN (4)     // GPIO 4  (13)

// #define ESP3D_SD_DETECT_PIN (-1) //GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4000)

#define SPI_ALLOCATION_SIZE (16 * 1024)
//(range 400kHz - 20MHz for SDSPI, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (20000)

#define SD_SPI_HOST SPI2_HOST  // 1
// #define SD_SPI_HOST SPI3_HOST  // 2

#ifdef __cplusplus
} /* extern "C" */
#endif
