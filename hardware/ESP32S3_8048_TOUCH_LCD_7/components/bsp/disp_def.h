// Display definitions for ESP32S3_8048_TOUCH_LCD_7
// Display driver ST7262 (16-bit RGB565 parallel interface)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7262"

#include "st7262.h"

#define DISP_HOR_RES_MAX 800
#define DISP_VER_RES_MAX 480

#define DISP_CLK_FREQ (18 * 1000 * 1000)
#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER (false)
#define DISP_USE_DOUBLE_BUFFER (true)
#define DISP_NUM_FB (1)

#define DISP_PATCH_FS_FREQ (6 * 1000 * 1000)  // 6MHz
#define DISP_PATCH_FS_DELAY (40)

#if DISP_NUM_FB == 2
// Full frame buffer (255KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX)
#else
// 1/4 (68-line) buffer (63.75KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 4)
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * 2)

const esp_rgb_st7262_config_t disp_panel_cfg = {
    .panel_config =
        {.clk_src = LCD_CLK_SRC_DEFAULT,
         .timings =
             {
                 .pclk_hz = DISP_CLK_FREQ,
                 .h_res = DISP_HOR_RES_MAX,
                 .v_res = DISP_VER_RES_MAX,
                 .hsync_pulse_width = 4,
                 .hsync_back_porch = 8,
                 .hsync_front_porch = 8,
                 .vsync_pulse_width = 4,
                 .vsync_back_porch = 16,
                 .vsync_front_porch = 16,
                 .flags =
                     {
                         .hsync_idle_low = 0,
                         .vsync_idle_low = 0,
                         .de_idle_high = 0,
                         .pclk_active_neg = true,
                         .pclk_idle_high = 0,
                     },
             },
         .data_width = 16,  // RGB565 in parallel mode
         .bits_per_pixel = 0,
         .num_fbs = DISP_NUM_FB,
#if DISP_USE_BOUNCE_BUFFER
         .bounce_buffer_size_px = DISP_BUF_SIZE,
#else
         .bounce_buffer_size_px = 0,
#endif
         .sram_trans_align = 0,
         .psram_trans_align = 64,
         .hsync_gpio_num = 46,  // GPIO 46
         .vsync_gpio_num = 3,   // GPIO 3
         .de_gpio_num = 5,     // GPIO 5
         .pclk_gpio_num = 7,   // GPIO 7
         .disp_gpio_num = -1,   // EN pin not connected
         .data_gpio_nums =
             {
                 14,   // D0  (B0) - GPIO 14
                 38,   // D1  (B1) - GPIO 38
                 18,   // D2  (B2) - GPIO 18
                 17,   // D3  (B3) - GPIO 17
                 10,   // D4  (B4) - GPIO 10
                 39,   // D5  (G0) - GPIO 39
                 0,    // D6  (G1) - GPIO 0
                 45,   // D7  (G2) - GPIO 45
                 48,   // D8  (G3) - GPIO 48
                 47,   // D9  (G4) - GPIO 47
                 21,   // D10 (G5) - GPIO 21
                  1,   // D11 (R0) - GPIO 1
                  2,   // D12 (R1) - GPIO 2
                 42,   // D13 (R2) - GPIO 42
                 41,   // D14 (R3) - GPIO 41
                 40,   // D15 (R4) - GPIO 40
             },
         .flags =
             {
                 .disp_active_low = (uint32_t)NULL,
                 .refresh_on_demand = (uint32_t)NULL,
                 .fb_in_psram =
                     true,  // Do not change this, as it is mandatory for RGB
                            // parallel interface and octal PSRAM
                 .double_fb = (uint32_t)NULL,
                 .no_fb = (uint32_t)NULL,
                 .bb_invalidate_cache = (uint32_t)NULL,
             }},
    .orientation = orientation_landscape,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

#ifdef __cplusplus
} /* extern "C" */
#endif
