// Touch definitions for ESP32_3248S035R
// Touch driver XPT2046 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "XPT2046"

#include <driver/spi_master.h>

#include "xpt2046.h"

// Touch configuration
xpt2046_config_t xpt2046_cfg = {
    .irq_pin = 36,           // GPIO 36
    .touch_threshold = 300,  // Threshold for touch detection
    .swap_xy = true,
    .invert_x = false,
    .invert_y = false,
    .x_max = 480,
    .y_max = 320,
    .calibration_x_min = 140,
    .calibration_y_min = 290,
    .calibration_x_max = 3950,
    .calibration_y_max = 3890,
};

// SPI (shared with Display)
const spi_device_interface_config_t touch_spi_cfg = {
    .clock_speed_hz = 2 * 1000 * 1000,
    .mode = 0,
    .spics_io_num = 33,  // GPIO 33
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
