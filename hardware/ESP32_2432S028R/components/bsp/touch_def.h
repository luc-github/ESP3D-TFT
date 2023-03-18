//Pins definition for ESP32 2332S028R
//Touch driver XPT2046 No SPI
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#define XPT2046_X_INV           1
#define XPT2046_Y_INV           1
#define XPT2046_XY_SWAP		    0
#define XPT2046_TOUCH_IRQ       36 //GPIO 36
#define XPT2046_TOUCH_IRQ_PRESS 1
#define XPT2046_TOUCH_PRESS     1

#define TOUCH_SPI_MOSI 32 //GPIO 32
#define TOUCH_SPI_MISO 39 //GPIO 39
#define TOUCH_SPI_CLK  25 //GPIO 25  
#define TOUCH_SPI_CS   33 //GPIO 33

#ifdef __cplusplus
} /* extern "C" */
#endif