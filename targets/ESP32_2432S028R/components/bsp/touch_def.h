//Pins definition for ESP32 2332S028R
//Touch driver XPT2046 SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#define XPT2046_AVG             4
#define XPT2046_X_MIN           200
#define XPT2046_Y_MIN           100
#define XPT2046_X_MAX           1900
#define XPT2046_Y_MAX           1900
#define XPT2046_X_INV           1
#define XPT2046_Y_INV           1
#define XPT2046_XY_SWAP		    0
#define XPT2046_TOUCH_THRESHOLD 400 // Threshold for touch detection
#define XPT2046_TOUCH_IRQ       36 //GPIO 36
#define XPT2046_TOUCH_IRQ_PRESS 1
#define XPT2046_TOUCH_PRESS     1

#define TOUCH_SPI_MOSI 32 //GPIO 32
#define TOUCH_SPI_MISO 39 //GPIO 39
#define TOUCH_SPI_CLK  25 //GPIO 25  
#define TOUCH_SPI_CS   33 //GPIO 33

#define TOUCH_SPI_HOST SPI3_HOST
//#define TOUCH_SPI_HOST SPI2_HOST

#define TOUCH_SPI_CLOCK_SPEED_HZ    (2*1000*1000)
#define TOUCH_SPI_MODE          (0)

#ifdef __cplusplus
} /* extern "C" */
#endif