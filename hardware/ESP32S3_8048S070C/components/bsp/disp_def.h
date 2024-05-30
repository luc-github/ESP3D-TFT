// Display definitions for ESP32_8048S070C
// Display driver EK9716 (16-bit RGB565 parallel interface)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "EK9716"
#include "disp_backlight.h"
#include "ek9716.h"

#define DISP_HOR_RES_MAX 800
#define DISP_VER_RES_MAX 480

#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER (false)

#define DISP_USE_DOUBLE_BUFFER (true)
#define DISP_NUM_FB (1)

#define DISP_CLK_FREQ (16 * 1000 * 1000)

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

// Panel configuration
const esp_rgb_ek9716_config_t disp_panel_cfg = {
    .panel_config =
        {.clk_src = LCD_CLK_SRC_DEFAULT,
         .timings =
             {
                 .pclk_hz = DISP_CLK_FREQ,
                 .h_res = DISP_HOR_RES_MAX,
                 .v_res = DISP_VER_RES_MAX,
                 .hsync_pulse_width = 30,
                 .hsync_back_porch = 16,
                 .hsync_front_porch = 210,
                 .vsync_pulse_width = 13,
                 .vsync_back_porch = 10,
                 .vsync_front_porch = 22,
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
         .hsync_gpio_num = 39,  // GPIO 39
         .vsync_gpio_num = 40,  // GPIO 40
         .de_gpio_num = 41,     // GPIO 41
         .pclk_gpio_num = 42,   // GPIO 42
         .disp_gpio_num = -1,   // EN pin not connected
         .data_gpio_nums =
             {
                 15,  // D0  (B0) - GPIO 15
                 7,   // D1  (B1) - GPIO 7
                 6,   // D2  (B2) - GPIO 6
                 5,   // D3  (B3) - GPIO 5
                 4,   // D4  (B4) - GPIO 4
                 9,   // D5  (G0) - GPIO 9
                 46,  // D6  (G1) - GPIO 46
                 3,   // D7  (G2) - GPIO 3
                 8,   // D8  (G3) - GPIO 8
                 16,  // D9  (G4) - GPIO 16
                 1,   // D10 (G5) - GPIO 1
                 14,  // D11 (R0) - GPIO 14
                 21,  // D12 (R1) - GPIO 21
                 47,  // D13 (R2) - GPIO 47
                 48,  // D14 (R3) - GPIO 48
                 45,  // D15 (R4) - GPIO 45
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

#define DISP_BCKL_DEFAULT_DUTY 20  //%

const disp_backlight_config_t disp_bcklt_cfg = {.pwm_control = true,
                                                .output_invert = false,
                                                .gpio_num = 2,
                                                .timer_idx = 0,
                                                .channel_idx = 0};

#ifdef __cplusplus
} /* extern "C" */
#endif
