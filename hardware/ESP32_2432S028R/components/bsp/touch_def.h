// Touch definitions for ESP32_2432S028R
// Touch driver XPT2046 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#include "xpt2046.h"
#include "sw_spi.h"

// X/Y Calibration Values
#define TOUCH_X_MIN           380
#define TOUCH_Y_MIN           200
#define TOUCH_X_MAX           3950
#define TOUCH_Y_MAX           3850

// Touch orientation
xpt2046_config_t xpt2046_cfg = {
    .irq_pin = 36, // GPIO 36  
    .touch_threshold = 300, // Threshold for touch detection  
    .swap_xy = true,
    .invert_x = true,
    .invert_y = true,
};

// SPI (BitBang)
const sw_spi_config_t touch_spi_cfg = {
    .cs_pin = 33,   // GPIO 33
    .clk_pin = 25,  // GPIO 25
    .mosi_pin = 32, // GPIO 33
    .miso_pin = 39, // GPIO 39
};

#ifdef __cplusplus
} /* extern "C" */
#endif
