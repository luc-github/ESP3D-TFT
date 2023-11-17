// Pins definition for ESP32 ESP32S3_BZM_TFT35_GT911
// SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32S3_BZM_TFT35_GT911 SPI GPIO definition
 *
 */
#define ESP3D_SD_IS_SPI 1
#define ESP3D_SD_MOSI_PIN (47)  // GPIO 47
#define ESP3D_SD_MISO_PIN (14)  // GPIO 14
#define ESP3D_SD_CLK_PIN (21)   // GPIO 21
#define ESP3D_SD_CS_PIN (48)    // GPIO 48

#define SPI_ALLOCATION_SIZE (16 * 1024)

// #define ESP3D_SD_DETECT_PIN (-1) //GPIO -1
// #define ESP3D_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4092) 
//(range 400kHz - 20MHz for SDSPI, less for other devices)
// default is 20MHz
#define ESP3D_SD_FREQ (20000)

#define SD_SPI_HOST SPI2_HOST //1

#ifdef __cplusplus
} /* extern "C" */
#endif
