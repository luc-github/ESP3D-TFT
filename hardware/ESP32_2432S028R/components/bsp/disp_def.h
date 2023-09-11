// Display definitions for ESP32_2432S028R
// Display driver ILI9341 (SPI)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ILI9341"

#include "spi_bus.h"
#include "disp_spi.h"
#include "ili9341.h"
#include "disp_backlight.h"

/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/

#define DISP_ORIENTATION 3  // landscape inverted

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 240
#else  // portrait mode
#define DISP_HOR_RES_MAX 240
#define DISP_VER_RES_MAX 320
#endif

#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
  // 1/10 (24-line) buffer (15KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_SPIRAM
#else
  // 1/24 (10-line) buffer (6.25KB) in internal DRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

ili9341_config_t ili9341_cfg = {
    .rst_pin = 4, // GPIO 4
    .dc_pin = 2, // GPIO 2
};

// SPI (dedicated)
#define DISP_SPI_HOST SPI2_HOST  // 1
// #define DISP_SPI_HOST SPI3_HOST //2

#define DISP_SPI_CLK  14  // GPIO 14
#define DISP_SPI_MOSI 13  // GPIO 13
//#define DISP_SPI_MISO 12  // GPIO 12

spi_device_interface_config_t disp_spi_cfg = {
    .clock_speed_hz = 40 * 1000 * 1000,
    .mode = 0,
    .spics_io_num = 15, // GPIO 15
    .input_delay_ns = 0,
};

const disp_backlight_config_t disp_bcklt_cfg = {
    .gpio_num = 21, // GPIO 21
    .pwm_control = false,
    .output_invert = false,
};

#ifdef __cplusplus
} /* extern "C" */
#endif
