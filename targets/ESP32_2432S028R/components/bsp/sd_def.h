//Pins definition for ESP32_2432S028R
//SD SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32_2432S028R SD SPI GPIO definition
 *
 */
#define ESP_SD_IS_SPI   1
#define ESP_SD_MOSI_PIN    (23) //GPIO 35
#define ESP_SD_MISO_PIN    (19) //GPIO 19
#define ESP_SD_CLK_PIN     (18) //GPIO 18
#define ESP_SD_CS_PIN      (5) //GPIO 5

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
