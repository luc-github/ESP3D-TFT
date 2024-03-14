// Display definitions for ESP32_2432S028R
// Display driver ILI9341 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ILI9341"

#include "disp_backlight.h"
#include "ili9341.h"
#include "spi_bus.h"


// Display orientation
/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/
#define DISP_ORIENTATION 3  // landscape inverted

// Display resolution
#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 240
#else  // portrait mode
#define DISP_HOR_RES_MAX 240
#define DISP_VER_RES_MAX 320
#endif

// Display interface
#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
// 1/10 (24-line) buffer (15KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_SPIRAM
#else
// 1/20 (12-line) buffer (7.5KB) in internal DRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 12)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * 2)

// LCD panel configuration
esp_lcd_panel_dev_config_t disp_panel_cfg = {
    .reset_gpio_num = 4,  // GPIO 4
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    .bits_per_pixel = 16,
    .flags = {.reset_active_high = 0},
    .vendor_config = NULL};

// SPI (dedicated)
#define DISP_SPI_HOST SPI2_HOST  // 1

// SPI pins definition (common)
#define DISP_SPI_CLK 14   // GPIO 14
#define DISP_SPI_MOSI 13  // GPIO 13
#define DISP_SPI_MISO 12  // GPIO 12

// SPI definition for LCD
esp_lcd_panel_io_spi_config_t disp_spi_cfg = {
    .cs_gpio_num = 15, /*!< GPIO used for CS line */
    .dc_gpio_num = 2,  /*!< GPIO used to select the D/C line, set this to -1 if
                          the D/C line is not used */
    .spi_mode = 0,     /*!< Traditional SPI mode (0~3) */
    .pclk_hz = 40 * 1000 * 1000, /*!< Frequency of pixel clock */
    .trans_queue_depth = 10,     /*!< Size of internal transaction queue */
    .on_color_trans_done =
        NULL, /*!< Callback invoked when color data transfer has finished */
    .user_ctx = NULL,    /*!< User private data, passed directly to
                            on_color_trans_done's user_ctx */
    .lcd_cmd_bits = 8,   /*!< Bit-width of LCD command */
    .lcd_param_bits = 8, /*!< Bit-width of LCD parameter */
    .flags = {
        /*!< Extra flags to fine-tune the SPI device */
        .dc_low_on_data =
            0, /*!< If this flag is enabled, DC line = 0 means transfer data, DC
                  line = 1 means transfer command; vice versa */
        .octal_mode = 0, /*!< transmit with octal mode (8 data lines), this mode
                            is used to simulate Intel 8080 timing */
        .quad_mode = 0,  /*!< transmit with quad mode (4 data lines), this mode
                            is useful when transmitting LCD parameters (Only use
                            one line for command) */
        .sio_mode = 0,  /*!< Read and write through a single data line (MOSI) */
        .lsb_first = 0, /*!< transmit LSB bit first */
        .cs_high_active = 0, /*!< CS line is high active */
    }};

// Backlight configuration
#define DISP_BCKL_DEFAULT_DUTY 100  //%

const disp_backlight_config_t disp_bcklt_cfg = {.pwm_control = false,
                                                .output_invert = false,
                                                .gpio_num = 21,
                                                .timer_idx = 0,
                                                .channel_idx = 0};

#ifdef __cplusplus
} /* extern "C" */
#endif
