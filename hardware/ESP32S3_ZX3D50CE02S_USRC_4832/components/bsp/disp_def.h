// Pins for ESP32S3_ZX3D50CE02S_USRC_4832
// Display driver ST7796 parallele 8080
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "st7796.h"
#include "disp_backlight.h"

#define TFT_DISPLAY_CONTROLLER "ST7796"

#define DISP_ORIENTATION \
  (orientation_landscape)  // 0:portrait mode 1:landscape mode 2:portrait mode
                           // reverse 3:landscape mode reverse

#if DISP_ORIENTATION == orientation_landscape || \
    DISP_ORIENTATION == orientation_landscape_invert  // landscape mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (320)
#else  // portrait mode
#define DISP_HOR_RES_MAX (320)
#define DISP_VER_RES_MAX (480)
#endif

#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * (DISP_VER_RES_MAX / 10))

const esp_i80_st7796_config_t st7796_cfg = {
    .bus_config =
        {
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .dc_gpio_num = 0,  // DISP_RS_PIN=0
            .wr_gpio_num = 47,  // DISP_WR_PIN=GPIO47
            .data_gpio_nums =
                {
                    9,   // DISP_D00_PIN = GPIO9
                    46,   // DISP_D01_PIN = GPIO46
                    3,   // DISP_D02_PIN = GPIO3
                    8,  // DISP_D03_PIN = GPIO8
                    18,   // DISP_D04_PIN = GPIO18
                    17,  // DISP_D05_PIN = GPIO17
                    16,   // DISP_D06_PIN = GPIO16
                    15,  // DISP_D07_PIN = GPIO15
                    -1,   // DISP_D08_PIN = N/C
                    -1,  // DISP_D09_PIN = N/C
                    -1,   // DISP_D10_PIN = N/C
                    -1,  // DISP_D11_PIN = N/C
                    -1,   // DISP_D12_PIN = N/C
                    -1,  // DISP_D13_PIN = N/C
                    -1,   // DISP_D14_PIN = N/C
                    -1,  // DISP_D15_PIN = N/C
                },
            .bus_width = 8,  // DISP_BITS_WIDTH
            .max_transfer_bytes = DISP_BUF_SIZE * sizeof(uint16_t),
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        },
    .io_config =
        {
            .cs_gpio_num = 48, //DISP_CS_PIN
            .pclk_hz =
                (8 * 1000 *
                 1000),  // could be 10 if no PSRAM memory= DISP_CLK_FREQ,
            .trans_queue_depth = 10,
            .dc_levels =
                {
                    .dc_idle_level = 0,
                    .dc_cmd_level = 0,
                    .dc_dummy_level = 0,
                    .dc_data_level = 1,
                },
            .on_color_trans_done =
                NULL,  // Callback invoked when color data
                       // transfer has finished updated in bdsp.c
            .user_ctx =
                NULL,  // User private data, passed directly to
                       // on_color_trans_done’s user_ctx updated in bdsp.c
            .lcd_cmd_bits = 8,    // DISP_CMD_BITS_WIDTH
            .lcd_param_bits = 8,  // DISP_PARAM_BITS_WIDTH
        },
    .panel_config =
        {
            .reset_gpio_num = 4,  // DISP_RST_PIN = GPIO4 - Same as touch
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
            .data_endian =
                0, /*!< Set the data endian for color data larger than 1 byte */
            .bits_per_pixel = 16, /*!< Color depth, in bpp */
            .flags =
                {
                    .reset_active_high = 0, /*!< Setting this if the panel reset
                                               is high level active */
                },
            .vendor_config = NULL, /*!< Vendor specific configuration */
        },
    .orientation = DISP_ORIENTATION,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

// Display backlight configuration
#define DISP_BCKL_DEFAULT_DUTY 100  //%

const disp_backlight_config_t disp_bcklt_cfg = {
    .pwm_control = false,
    .output_invert = false,
    .gpio_num = 45, // DISP_BL_PIN=GPIO45 
    .timer_idx = 0,
    .channel_idx = 0
};

#ifdef __cplusplus
} /* extern "C" */
#endif