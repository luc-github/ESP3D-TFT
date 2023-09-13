// Touch definitions for: ESP32_3248S035C, ESP32_3248S035R, ESP32_2432S028R
// Touch drivers: GT911 (I2C), XPT2046 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// --------------------- Touch I2C/SPI settings ---------------------
#if TARGET_ESP32_3248S035C
  #include "i2c_def.h"
#elif TARGET_ESP32_3248S035R
  // SPI (shared with Display)
  #include "shared_spi_def.h"
  
  #define TOUCH_SPI_HOST  SHARED_SPI_HOST

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
#elif TARGET_ESP32_2432S028R
  // SPI (BitBang)
  #define TOUCH_SW_SPI    1
  #include "sw_spi.h"
    
  const sw_spi_config_t touch_spi_cfg = {
      .cs_pin = 33,   // GPIO 33
      .clk_pin = 25,  // GPIO 25
      .mosi_pin = 32, // GPIO 33
      .miso_pin = 39, // GPIO 39
  };
#endif

// ------------------- Touch Controller settings -------------------
#if TARGET_ESP32_3248S035C
  #define TOUCH_CONTROLLER    "GT911"
  #define TOUCH_GT911         1
  #include "gt911.h"

  const gt911_config_t gt911_cfg = {
      .i2c_clk_speed = 400*1000,
      .rst_pin = 25, // GPIO 25
    #if WITH_GT911_INT
      .int_pin = 21, // GPIO 21
    #else
      .int_pin = -1, // INT pin not connected (by default)
    #endif
      .swap_xy = true,
      .invert_x = true,
      .invert_y = false,
  };
#elif TARGET_ESP32_3248S035R || TARGET_ESP32_2432S028R
  #define TOUCH_CONTROLLER    "XPT2046"
  #define TOUCH_XPT2046        1
  #include "xpt2046.h"

  xpt2046_config_t xpt2046_cfg = {
      .irq_pin = 36, // GPIO 36  
      .touch_threshold = 300, // Threshold for touch detection
      .swap_xy = true,
      .invert_x = true,
      .invert_y = true,
  };
#endif

// --------------------- X/Y Calibration values ---------------------
#if TARGET_ESP32_3248S035R
  #define TOUCH_X_MIN   140
  #define TOUCH_Y_MIN   290
  #define TOUCH_X_MAX   3950
  #define TOUCH_Y_MAX   3890
#elif TARGET_ESP32_2432S028R
  #define TOUCH_X_MIN   380
  #define TOUCH_Y_MIN   200
  #define TOUCH_X_MAX   3950
  #define TOUCH_Y_MAX   3850
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
