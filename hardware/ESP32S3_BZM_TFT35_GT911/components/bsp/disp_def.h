// Pins for ESP32S3_BZM_TFT35_GT911
// Display driver ST7796 parallele 8080
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

#include "disp_backlight.h"
#include "spi_bus.h"
#include "st7796.h"

#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (320)

#if WITH_PSRAM
// 1/10 (32-line) buffer (30KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#else
// 1/40 (8-line) buffer (7.5KB) in internal DRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * sizeof(uint16_t))

// LCD panel configuration
esp_spi_st7262_config_t display_spi_st7262_cfg = {
    .panel_dev_config = {.reset_gpio_num = 45,
                         .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
                         .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
                         .bits_per_pixel = 16,
                         .flags = {.reset_active_high = 0},
                         .vendor_config = NULL},

    .spi_bus_config =
        {
            .spi_host_index = SPI3_HOST,
            .pin_miso = -1,    /**< MISO pin number */
            .pin_mosi = 38,    /**< MOSI pin number */
            .pin_clk = 39,     /**< CLK pin number */
            .is_master = true, /**< SPI master mode */
            .max_transfer_sz =
                DISP_BUF_SIZE_BYTES,        /**< Maximum transfer size */
            .dma_channel = SPI_DMA_CH_AUTO, /**< DMA channel */
            .quadwp_io_num = -1,            /**< QuadWP pin number */
            .quadhd_io_num = -1             /**< QuadHD pin number */
        },
    .disp_spi_cfg =
        {
            .cs_gpio_num = 41, /*!< GPIO used for CS line */
            .dc_gpio_num = 40, /*!< GPIO used to select the D/C line, set this
                                 to -1 if the D/C line is not used */
            .spi_mode = 0,     /*!< Traditional SPI mode (0~3) */
            .pclk_hz = 40 * 1000 * 1000, /*!< Frequency of pixel clock */
            .trans_queue_depth = 10, /*!< Size of internal transaction queue */
            .on_color_trans_done = NULL, /*!< Callback invoked when color data
                                            transfer has finished */
            .user_ctx = NULL,   /*!< User private data, passed directly to
                                   on_color_trans_done's user_ctx */
            .lcd_cmd_bits = 8,  /*!< Bit-width of LCD command */
            .lcd_param_bits = 8 /*!< Bit-width of LCD parameter */
        },
    .orientation = orientation_landscape,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

// Display backlight configuration
#define DISP_BCKL_DEFAULT_DUTY 100  //%

const disp_backlight_config_t disp_bcklt_cfg = {
    .pwm_control = false,
    .output_invert = false,
    .gpio_num = 48,  // DISP_BL_PIN=GPIO48
    .timer_idx = 0,
    .channel_idx = 0};

#ifdef __cplusplus
} /* extern "C" */
#endif
