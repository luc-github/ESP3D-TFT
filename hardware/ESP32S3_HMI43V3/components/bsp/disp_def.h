// Pins for ESP32S3 HMI43V3
// Display driver RM68120 parallele 8080
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "rm68120.h"
#define TFT_DISPLAY_CONTROLLER "RM68120"

#define DISP_HOR_RES_MAX (800)
#define DISP_VER_RES_MAX (480)


#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * (DISP_VER_RES_MAX / 10))
#define DISP_USE_DOUBLE_BUFFER (true)

const esp_i80_rm68120_config_t rm68120_cfg = {
    .bus_config =
        {
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .dc_gpio_num = 38,  // DISP_RS_PIN=GPIO38
            .wr_gpio_num = 17,  // DISP_WR_PIN=GPIO17
            .data_gpio_nums =
                {
                    1,   // DISP_D00_PIN = GPIO1
                    9,   // DISP_D01_PIN = GPIO9
                    2,   // DISP_D02_PIN = GPIO2
                    10,  // DISP_D03_PIN = GPIO10
                    3,   // DISP_D04_PIN = GPIO3
                    11,  // DISP_D05_PIN = GPIO11
                    4,   // DISP_D06_PIN = GPIO4
                    12,  // DISP_D07_PIN = GPIO12
                    5,   // DISP_D08_PIN = GPIO5
                    13,  // DISP_D09_PIN = GPIO13
                    6,   // DISP_D10_PIN = GPIO6
                    14,  // DISP_D11_PIN = GPIO14
                    7,   // DISP_D12_PIN = GPIO7
                    15,  // DISP_D13_PIN = GPIO15
                    8,   // DISP_D14_PIN = GPIO8
                    16,  // DISP_D15_PIN = GPIO16
                },
            .bus_width = 16,  // DISP_BITS_WIDTH
            .max_transfer_bytes = DISP_BUF_SIZE * sizeof(uint16_t),
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        },
    .io_config =
        {
            .cs_gpio_num = -1,
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
            .lcd_cmd_bits = 16,    // DISP_CMD_BITS_WIDTH
            .lcd_param_bits = 16,  // DISP_PARAM_BITS_WIDTH
        },
    .panel_config =
        {
            .reset_gpio_num = 21,  // DISP_RST_PIN = GPIO21
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
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
    .orientation = orientation_landscape,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

#ifdef __cplusplus
} /* extern "C" */
#endif
