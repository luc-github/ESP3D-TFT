// Touch definitions for ESP32_2432S028R
// Touch driver XPT2046 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#include "sw_spi.h"
#include "xpt2046.h"

xpt2046_config_t xpt2046_cfg = {
    .irq_pin = 36,           // GPIO 36
    .touch_threshold = 300,  // Threshold for touch detection
    .swap_xy = true,
    .invert_x = false,
    .invert_y = false,
    .x_max = 320,
    .y_max = 240,
    .calibration_x_min = 380,
    .calibration_y_min = 200,
    .calibration_x_max = 3950,
    .calibration_y_max = 3850,
};

// SPI (BitBang)
const sw_spi_config_t touch_spi_cfg = {
    .cs_pin = 33,    // GPIO 33
    .clk_pin = 25,   // GPIO 25
    .mosi_pin = 32,  // GPIO 33
    .miso_pin = 39,  // GPIO 39
};

#ifdef __cplusplus
} /* extern "C" */
#endif
