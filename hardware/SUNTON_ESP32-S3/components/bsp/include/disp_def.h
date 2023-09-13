// Display definitions for: ESP32_8048S070C, ESP32_8048S050C, ESP32_8048S043C, ESP32_4827S043C
// Display drivers: EK9716, ST7262, ILI9485 (16-bit RGB565 parallel interface)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_ESP32S3_8048S070C
  #define TFT_DISPLAY_CONTROLLER "EK9716"
#elif TARGET_ESP32S3_8048S050C || TARGET_ESP32S3_8048S043C
  #define TFT_DISPLAY_CONTROLLER "ST7262"
#elif TARGET_ESP32S3_4827S043C
  #define TFT_DISPLAY_CONTROLLER "ILI9485"
#endif

#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_rgb.h"

#include "disp_res_def.h"

#if TARGET_ESP32S3_8048S070C
  #define DISP_CLK_FREQ       (16 * 1000 * 1000)
#elif TARGET_ESP32S3_8048S043C
  #define DISP_CLK_FREQ       (13 * 1000 * 1000)  // adjusted
#else
  #define DISP_CLK_FREQ       (14 * 1000 * 1000)
#endif

#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER  (false)
#define DISP_USE_DOUBLE_BUFFER  (true)
#define DISP_NUM_FB             (1)

#if DISP_NUM_FB == 2
  // 800x480: Full frame buffer (750KB) in external PSRAM
  // 480x272: Full frame buffer (255KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX)
#else
  // 800x480: 1/4 (120-line) buffer (187.5KB) in external PSRAM
  // 480x272: 1/4 (68-line) buffer (63.75KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 4)
#endif

#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

const esp_lcd_rgb_panel_config_t disp_panel_cfg = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .timings = {
        .pclk_hz = DISP_CLK_FREQ,
        .h_res = DISP_HOR_RES_MAX,
        .v_res = DISP_VER_RES_MAX,
      #if TARGET_ESP32S3_8048S070C
        .hsync_pulse_width = 30,
        .hsync_back_porch = 16,
        .hsync_front_porch = 210,
        .vsync_pulse_width = 13,
        .vsync_back_porch = 10,
        .vsync_front_porch = 22,
      #else
        .hsync_pulse_width = 4,
        .hsync_back_porch = 8,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 8,
        .vsync_front_porch = 8,
      #endif
        .flags = {
            .hsync_idle_low = 0,
            .vsync_idle_low = 0,
            .de_idle_high = 0,
            .pclk_active_neg = true,
            .pclk_idle_high = 0,
        },
    },
    .data_width = 16, // RGB565 in parallel mode
    .bits_per_pixel = 0,
    .num_fbs = DISP_NUM_FB,
  #if DISP_USE_BOUNCE_BUFFER
    .bounce_buffer_size_px = DISP_BUF_SIZE,
  #else
    .bounce_buffer_size_px = 0,
  #endif
    .sram_trans_align = 0,
    .psram_trans_align = 64,
    .hsync_gpio_num = 39, // GPIO 39
  #if TARGET_ESP32S3_8048S070C
    .vsync_gpio_num = 40, // GPIO 40
    .de_gpio_num = 41, // GPIO 41
  #else
    .vsync_gpio_num = 41, // GPIO 41
    .de_gpio_num = 40, // GPIO 40
  #endif
    .pclk_gpio_num = 42, // GPIO 42
    .disp_gpio_num = -1, // EN pin not connected
    .data_gpio_nums = {
      #if TARGET_ESP32S3_8048S070C
        15, // D0  (B0) - GPIO 15
        7,  // D1  (B1) - GPIO 7
        6,  // D2  (B2) - GPIO 6
        5,  // D3  (B3) - GPIO 5
        4,  // D4  (B4) - GPIO 4
        9,  // D5  (G0) - GPIO 9
        46, // D6  (G1) - GPIO 46
        3,  // D7  (G2) - GPIO 3
        8,  // D8  (G3) - GPIO 8
        16, // D9  (G4) - GPIO 16
        1,  // D10 (G5) - GPIO 1
        14, // D11 (R0) - GPIO 14
        21, // D12 (R1) - GPIO 21
        47, // D13 (R2) - GPIO 47
        48, // D14 (R3) - GPIO 48
        45, // D15 (R4) - GPIO 45
      #else
        8,  // D0  (B0) - GPIO 8
        3,  // D1  (B1) - GPIO 3
        46, // D2  (B2) - GPIO 46
        9,  // D3  (B3) - GPIO 9
        1,  // D4  (B4) - GPIO 1
        5,  // D5  (G0) - GPIO 5
        6,  // D6  (G1) - GPIO 6
        7,  // D7  (G2) - GPIO 7
        15, // D8  (G3) - GPIO 15
        16, // D9  (G4) - GPIO 16
        4,  // D10 (G5) - GPIO 4
        45, // D11 (R0) - GPIO 45
        48, // D12 (R1) - GPIO 48
        47, // D13 (R2) - GPIO 47
        21, // D14 (R3) - GPIO 21
        14, // D15 (R4) - GPIO 14
      #endif
    },
    .flags = {
        .disp_active_low = (uint32_t)NULL,
        .refresh_on_demand = (uint32_t)NULL,
        .fb_in_psram = true, // Do not change this, as it is mandatory for RGB parallel interface and octal PSRAM
        .double_fb = (uint32_t)NULL,
        .no_fb = (uint32_t)NULL,
        .bb_invalidate_cache = (uint32_t)NULL,
    }
};

// ------------------- Display Backlight settings -------------------
#include "disp_backlight.h"

#define DISP_BCKL_DEFAULT_DUTY 20  // %

const disp_backlight_config_t disp_bcklt_cfg = {
    .gpio_num = 2, // GPIO 2
    .pwm_control = true,
    .output_invert = false,
    .timer_idx = 0,
    .channel_idx = 0
};

#ifdef __cplusplus
} /* extern "C" */
#endif
