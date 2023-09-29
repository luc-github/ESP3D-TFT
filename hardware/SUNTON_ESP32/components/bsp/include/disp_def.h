// Display definitions for: ESP32_3248S035C, ESP32_3248S035R, ESP32_2432S028R
// Display drivers: ST7796 (SPI), ILI9341 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// ------------------- Display SPI host settings -------------------
#if TARGET_ESP32_3248S035C || TARGET_ESP32_2432S028R
  // SPI (dedicated)
  #include "spi_bus.h"

  #define DISP_SPI_HOST SPI2_HOST  // 1
  // #define DISP_SPI_HOST SPI3_HOST //2

  #define DISP_SPI_CLK  14  // GPIO 14
  #define DISP_SPI_MOSI 13  // GPIO 13
  //#define DISP_SPI_MISO 12  // GPIO 12
#elif TARGET_ESP32_3248S035R
  // SPI (shared with Touch)
  #include "shared_spi_def.h"

  #define DISP_SPI_HOST  SHARED_SPI_HOST
#endif

// ---------------- Display SPI device configuration ----------------
#include "esp_lcd_panel_io.h"

esp_lcd_panel_io_spi_config_t disp_spi_cfg = {
    .dc_gpio_num = 2, // GPIO 2
    .cs_gpio_num = 15, // GPIO 15
    .pclk_hz = 40 * 1000 * 1000,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .spi_mode = 0,
    .trans_queue_depth = 10,
};

// ------------------ Display Controller settings ------------------
#if TARGET_ESP32_3248S035C || TARGET_ESP32_3248S035R
  #define TFT_DISPLAY_CONTROLLER "ST7796"
  #define DISP_ST7796   1
  #include "st7796.h"
#elif TARGET_ESP32_2432S028R
  #define TFT_DISPLAY_CONTROLLER "ILI9341"
  #define DISP_ILI9341  1
  #include "ili9341.h"
#endif

esp_lcd_panel_dev_config_t disp_panel_cfg = {
  #if TARGET_ESP32_3248S035C || TARGET_ESP32_3248S035R
    .reset_gpio_num = -1, // st7796 RST not connected to GPIO
  #elif TARGET_ESP32_2432S028R
    .reset_gpio_num = 4, // GPIO 4
  #endif
    .color_space = ESP_LCD_COLOR_SPACE_BGR,
    .bits_per_pixel = 16,
};

// --------------- Display Resolution/Buffer settings ---------------
#include "disp_res_def.h"

#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
  // 480x320: 1/10 (32-line) buffer (30KB) in external PSRAM
  // 320x240: 1/10 (24-line) buffer (15KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_SPIRAM
#else
  // 480x320: 1/32 (10-line) buffer (9.38KB) in internal DRAM
  // 320x240: 1/24 (10-line) buffer (6.25KB) in internal DRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_DMA
#endif  // WITH_PSRAM

#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

// ------------------- Display Backlight settings -------------------
#include "disp_backlight.h"

#define DISP_BCKL_DEFAULT_DUTY 100  // %

const disp_backlight_config_t disp_bcklt_cfg = {
  #if TARGET_ESP32_3248S035C || TARGET_ESP32_3248S035R
    .gpio_num = 27, // GPIO 27
  #elif TARGET_ESP32_2432S028R
    .gpio_num = 21, // GPIO 21  
  #endif
    .pwm_control = false,
    .output_invert = false,
    .timer_idx = 0,
    .channel_idx = 0
};

#ifdef __cplusplus
} /* extern "C" */
#endif
