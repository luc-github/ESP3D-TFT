// Display definitions for ESP32_3248S035R
// Display driver ST7796 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

#include "st7796.h"
#include "disp_backlight.h"

/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/

#define DISP_ORIENTATION 3  // landscape inverted

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 320
#else  // portrait mode
#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 480
#endif

#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
  // 1/10 (32-line) buffer (30KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_SPIRAM
#else
  // 1/40 (8-line) buffer (7.5KB) in internal DRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

esp_lcd_panel_dev_config_t disp_panel_cfg = {
    .reset_gpio_num = -1, // st7796 RST not connected to GPIO
    .color_space = ESP_LCD_COLOR_SPACE_BGR,
    .bits_per_pixel = 16,
};

// SPI (shared with Touch)
esp_lcd_panel_io_spi_config_t disp_spi_cfg = {
    .dc_gpio_num = 2, // GPIO 2
    .cs_gpio_num = 15, // GPIO 15
    .pclk_hz = 40 * 1000 * 1000,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .spi_mode = 0,
    .trans_queue_depth = 10,
};

const disp_backlight_config_t disp_bcklt_cfg = {
    .gpio_num = 27, // GPIO 27
    .pwm_control = false,
    .output_invert = false,
};

#ifdef __cplusplus
} /* extern "C" */
#endif
