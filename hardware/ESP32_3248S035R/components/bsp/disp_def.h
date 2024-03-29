// Display definitions for ESP32_3248S035R
// Display driver ST7796 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

#include "disp_backlight.h"
#include "spi_bus.h"
#include "st7796.h"

#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 320

#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
// 1/10 (32-line) buffer (30KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_SPIRAM
#else
// 1/40 (8-line) buffer (7.5KB) in internal DRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * 2)

// Display configuration
esp_spi_st7262_config_t display_spi_st7262_cfg = {
    .panel_dev_config = {.reset_gpio_num =
                             4,  // GPIO 4
                         .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
                         .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
                         .bits_per_pixel = 16,
                         .flags = {.reset_active_high = 0},
                         .vendor_config = NULL},

    .spi_bus_config =  // SPI (shared with Touch)
    {
        .spi_host_index = SPI2_HOST,
        .pin_miso = 12,                       /**< MISO pin number */
        .pin_mosi = 13,                       /**< MOSI pin number */
        .pin_clk = 14,                        /**< CLK pin number */
        .is_master = true,                    /**< SPI master mode */
        .max_transfer_sz = DISP_BUF_SIZE * 2, /**< Maximum transfer size */
        .dma_channel = 1,                     /**< DMA channel */
        .quadwp_io_num = -1,                  /**< QuadWP pin number */
        .quadhd_io_num = -1                   /**< QuadHD pin number */
    },
    .disp_spi_cfg =
        {.cs_gpio_num = 15, /*!< GPIO used for CS line */
         .dc_gpio_num = 2, /*!< GPIO used to select the D/C line, set this to -1
                              if the D/C line is not used */
         .spi_mode = 0,    /*!< Traditional SPI mode (0~3) */
         .pclk_hz = 40 * 1000 * 1000, /*!< Frequency of pixel clock */
         .trans_queue_depth = 10,     /*!< Size of internal transaction queue */
         .on_color_trans_done = NULL, /*!< Callback invoked when color data
                                         transfer has finished */
         .user_ctx = NULL,            /*!< User private data, passed directly to
                                         on_color_trans_done's user_ctx */
         .lcd_cmd_bits = 8,           /*!< Bit-width of LCD command */
         .lcd_param_bits = 8,         /*!< Bit-width of LCD parameter */
         .flags =
             {
                 /*!< Extra flags to fine-tune the SPI device */
                 .dc_low_on_data = 0, /*!< If this flag is enabled, DC line = 0
                                         means transfer data, DC line = 1 means
                                         transfer command; vice versa */
                 .octal_mode =
                     0, /*!< transmit with octal mode (8 data lines), this mode
                           is used to simulate Intel 8080 timing */
                 .quad_mode =
                     0, /*!< transmit with quad mode (4 data lines), this mode
                           is useful when transmitting LCD parameters (Only use
                           one line for command) */
                 .sio_mode =
                     0, /*!< Read and write through a single data line (MOSI) */
                 .lsb_first = 0,      /*!< transmit LSB bit first */
                 .cs_high_active = 0, /*!< CS line is high active */
             }},
    .orientation = orientation_landscape,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

// Backlight configuration
#define DISP_BCKL_DEFAULT_DUTY 100  //%

const disp_backlight_config_t disp_bcklt_cfg = {.pwm_control = false,
                                                .output_invert = false,
                                                .gpio_num = 27,
                                                .timer_idx = 0,
                                                .channel_idx = 0};

#ifdef __cplusplus
} /* extern "C" */
#endif
