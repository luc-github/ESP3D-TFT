// Touch definitions for ESP32_3248S035R
// Touch driver XPT2046 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#include "xpt2046.h"
#include <driver/spi_master.h>

// X/Y Calibration Values
#define TOUCH_X_MIN   140
#define TOUCH_Y_MIN   290
#define TOUCH_X_MAX   3950
#define TOUCH_Y_MAX   3890

xpt2046_config_t xpt2046_cfg = {
    .irq_pin = 36, // GPIO 36  
    .touch_threshold = 300, // Threshold for touch detection  
    .swap_xy = true,
    .invert_x = true,
    .invert_y = true,
};

// SPI (shared with Display)
const spi_device_interface_config_t touch_spi_cfg = {
    .clock_speed_hz = 2*1000*1000,
    .mode = 0,
    .spics_io_num = 33, // GPIO 33
    .queue_size = 1,
    .pre_cb = NULL,
    .post_cb = NULL,
    .command_bits = 8,
    .address_bits = 0,
    .dummy_bits = 0,
    .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_NO_DUMMY,
};

#ifdef __cplusplus
} /* extern "C" */
#endif
