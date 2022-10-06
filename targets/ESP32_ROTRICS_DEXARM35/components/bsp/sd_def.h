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
#define ESP_SDIO_CMD_PIN    (15) //GPIO 15
#define ESP_SDIO_CLK_PIN    (14) //GPIO 14
#define ESP_SDIO_D0_PIN     (2)  //GPIO 2
#define ESP_SDIO_D1_PIN     (-1) //NC 
#define ESP_SDIO_D2_PIN     (-1) //NC 
#define ESP_SDIO_D3_PIN     (-1) //NC 

#define ESP_SDIO_BIT_WIDTH 1 //can be 1 or 4


//#define ESP_SD_DETECT_PIN (-1) //GPIO -1
//#define ESP_SD_DETECT_VALUE (0) //LOW

//(range 400kHz - 40MHz for SDIO, less for other devices)
//default is 20MHz
#define ESP_SD_FREQ (20000)
#define ESP_SD_FORMAT_IF_MOUNT_FAILED   (1)


#ifdef __cplusplus
} /* extern "C" */
#endif
