// Display definitions for ESP32_8048S043C
// Display driver ST7262 (16-bit RGB565 parallel interface)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7262"

#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_rgb.h"
#include "disp_backlight.h"

/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/

#define DISP_ORIENTATION 2  // landscape

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 800
#define DISP_VER_RES_MAX 480
#else  // portrait mode
#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 800
#endif

#define DISP_CLK_FREQ           (13 * 1000 * 1000)  // adjusted
#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER  (false)
#define DISP_USE_DOUBLE_BUFFER  (true)
#define DISP_NUM_FB             (1)

#if DISP_NUM_FB == 2
  // Full frame buffer (255KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX)
#else
  // 1/4 (68-line) buffer (63.75KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 4)
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

const esp_lcd_rgb_panel_config_t disp_panel_cfg = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .timings = {
        .pclk_hz = DISP_CLK_FREQ,
        .h_res = DISP_HOR_RES_MAX,
        .v_res = DISP_VER_RES_MAX,
        .hsync_pulse_width = 4,
        .hsync_back_porch = 8,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 8,
        .vsync_front_porch = 8,
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
    .vsync_gpio_num = 41, // GPIO 41
    .de_gpio_num = 40, // GPIO 40
    .pclk_gpio_num = 42, // GPIO 42
    .disp_gpio_num = -1, // EN pin not connected
    .data_gpio_nums = {
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

#define DISP_BCKL_DEFAULT_DUTY 20  //%

const disp_backlight_config_t disp_bcklt_cfg = {
    .pwm_control = true,
    .output_invert = false, 
    .gpio_num = 2,
    .timer_idx = 0,
    .channel_idx = 0
};

#ifdef __cplusplus
} /* extern "C" */
#endif
