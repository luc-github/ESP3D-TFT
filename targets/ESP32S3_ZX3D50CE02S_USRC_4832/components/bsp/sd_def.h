//Pins definition for ESP32 ESP32S3_ZX3D50CE02S_USRC_4832
//SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32S3_ZX3D50CE02S_USRC_4832 SD SPI GPIO definition
 *
 */
#define ESP_SD_IS_SPI   1
#define ESP_SD_MOSI_PIN    (40) //GPIO 40
#define ESP_SD_MISO_PIN    (38) //GPIO 38
#define ESP_SD_CLK_PIN     (39) //GPIO 39
#define ESP_SD_CS_PIN      (41) //GPIO 41

//#define ESP_SD_DETECT_PIN (-1) //GPIO -1
//#define ESP_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4000)
//(range 400kHz - 20MHz for SDSPI, less for other devices)
//default is 20MHz
#define ESP_SD_FREQ (20000)
#define ESP_SD_SPI_DIV  (4)
#define ESP_SD_FORMAT_IF_MOUNT_FAILED   (1)


#ifdef __cplusplus
} /* extern "C" */
#endif
