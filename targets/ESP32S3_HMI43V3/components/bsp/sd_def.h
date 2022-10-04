//Pins definition for ESP32 ESP32S3_HMI43V3
//SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-S3-HMI-DevKit SD SPI GPIO definition
 *
 */
#define ESP_SD_IS_SPI   1
#define ESP_SD_MOSI_PIN    (35) //GPIO 35
#define ESP_SD_MISO_PIN    (37) //GPIO 37
#define ESP_SD_CLK_PIN     (36) //GPIO 36
#define ESP_SD_CS_PIN      (34) //GPIO 34

//#define ESP_SD_DETECT_PIN (-1) //GPIO -1
//#define ESP_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4000)
#define ESP_SD_FREQ (40000)
#define ESP_SD_SPI_DIV  (4)
#define ESP_SD_FORMAT_IF_MOUNT_FAILED   (1)


#ifdef __cplusplus
} /* extern "C" */
#endif
