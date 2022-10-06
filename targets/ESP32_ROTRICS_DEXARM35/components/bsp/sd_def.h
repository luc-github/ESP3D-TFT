//Pins definition for ESP32_ROTRICS_DEXARM35
//SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32_ROTRICS_DEXARM35 SD SPI GPIO definition
 *
 */
#define ESP_SD_IS_SPI   0
#define ESP_SD_IS_SDIO  1
//TODO
//DEFINE CMD DATA CLK PINS
#define ESP_SD_MOSI_PIN    (40) //GPIO 40


//#define ESP_SD_DETECT_PIN (-1) //GPIO -1
//#define ESP_SD_DETECT_VALUE (0) //LOW
#define MAX_TRANSFER_SZ (4000)
//(range 400kHz - 20MHz for SDSPI, less for other devices)
//default is 20MHz
#define ESP_SD_FREQ (20000)

#define ESP_SD_FORMAT_IF_MOUNT_FAILED   (1)


#ifdef __cplusplus
} /* extern "C" */
#endif
