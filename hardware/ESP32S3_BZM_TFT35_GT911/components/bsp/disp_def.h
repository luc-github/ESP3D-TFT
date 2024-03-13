// Display definitions for ESP32S3_BZM_TFT35_GT911
// Display driver ST7796 (SPI)
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

#include "spi_bus.h"
#include "st7796.h"
#include "disp_backlight.h"

#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 320

#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
// 1/10 (32-line) buffer (30KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#else
// 1/40 (8-line) buffer (7.5KB) in internal DRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#endif // WITH_PSRAM

#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * sizeof(uint16_t))

// SPI (dedicated)
#define DISP_SPI_HOST SPI3_HOST    // 2
#define DISP_SPI_CS 41             // GPIO 41
#define DISP_SPI_CLK 39            // GPIO 39
#define DISP_SPI_MOSI 38           // GPIO 38
                                   // #define DISP_SPI_MISO 12  // GPIO 12
#define DISP_RESET_PIN 45          // GPIO 45
#define DISP_DC_PIN 40             // GPIO 40
#define DISP_BCKL_DEFAULT_DUTY 100 //%

  const disp_backlight_config_t disp_bcklt_cfg = {
      .pwm_control = false,
      .output_invert = false,
      .gpio_num = 48, // GPIO 48
      .timer_idx = 0,
      .channel_idx = 0,
  };

#ifdef __cplusplus
} /* extern "C" */
#endif
