// Pins for ESP32 3248S035C
// Display driver ST7796 SPI
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

/*
PORTRAIT				0
PORTRAIT_INVERTED		1
LANDSCAPE				2
LANDSCAPE_INVERTED		3
*/

#define DISP_ORIENTATION 3  // landscape inverted

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (320)
#else  // portrait mode
#define DISP_HOR_RES_MAX (320)
#define DISP_VER_RES_MAX (480)
#endif

#if WITH_PSRAM
  // 1/10 (32-line) buffer (30KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_SPIRAM
#else
  // 1/40 (8-line) buffer (7.5KB) in internal DRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_DMA
#endif  // WITH_PSRAM

#define DISP_USE_DOUBLE_BUFFER 1

#define ST7796_INVERT_COLORS 0  // Flag

#define ST7796_RST -1           // (Not connected to GPIO)
#define ST7796_DC 2             // GPIO 2

#define DISP_SPI_MISO 12               // GPIO 12
#define DISP_SPI_MOSI 13               // GPIO 13
#define DISP_SPI_CLK 14                // GPIO 14
#define DISP_SPI_CS 15                 // GPIO 15

#define DISP_SPI_HOST SPI2_HOST  // 1
// #define DISP_SPI_HOST SPI3_HOST //2

#define DISP_BACKLIGHT_SWITCH 1
// #define DISP_BACKLIGHT_PWM //CONFIG_LV_DISP_BACKLIGHT_PWM

#define BACKLIGHT_ACTIVE_LVL 1  // CONFIG_LV_BACKLIGHT_ACTIVE_LVL
#define DISP_PIN_BCKL 27        // GPIO 27
#define DISP_BCKL_DEFAULT_DUTY 100
#define DISP_BL_ON (1)   // Screen backlight enabled
#define DISP_BL_OFF (0)  // Screen backlight enabled

#define DISP_SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)
#define DISP_SPI_CLOCK_SPEED_HZ (40 * 1000 * 1000)
#define DISP_SPI_MODE (0)

#define DISP_CMD_BITS_WIDTH (8)
#define DISP_PARAM_BITS_WIDTH (8)

#ifdef __cplusplus
} /* extern "C" */
#endif
