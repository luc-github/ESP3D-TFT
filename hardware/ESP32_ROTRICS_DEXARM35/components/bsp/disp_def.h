// Pins for ESP32 ROTRICS_DEXARM35
// Display driver ILI9488 SPI
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ILI9488"

#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 320

#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 32)

#define ILI9488_DC 27  // GPIO 27
// #define ILI9488_USE_RST   0
#define ILI9488_RST -1  // GPIO -1
#define ILI9488_INVERT_COLORS 0

/*
PORTRAIT				0
PORTRAIT_INVERTED		1
LANDSCAPE				2
LANDSCAPE_INVERTED		3
*/

#define ILI9488_DISPLAY_ORIENTATION 3  // landscape
#define DISP_SPI_MISO 34               // GPIO 34
#define DISP_SPI_MOSI 32               // GPIO 32
#define DISP_SPI_CLK 33                // GPIO 33
#define DISP_SPI_CS 19                 // GPIO 19
#define DISP_SPI_INPUT_DELAY_NS 0

#define DISP_SPI_HOST SPI2_HOST  // 1
// #define DISP_SPI_HOST SPI3_HOST //2

#define DISP_SPI_IO2 (-1)  // CONFIG_LV_DISP_SPI_IO2
#define DISP_SPI_IO3 (-1)  // CONFIG_LV_DISP_SPI_IO3

// #define DISP_SPI_HALF_DUPLEX //CONFIG_LV_TFT_DISPLAY_SPI_HALF_DUPLEX
#define DISP_SPI_FULL_DUPLEX

// #define DISP_SPI_TRANS_MODE_DIO //2-bit Dual SPI
// #define DISP_SPI_TRANS_MODE_QIO //4-bit Quad SPI
#define DISP_SPI_TRANS_MODE_SIO  // mosi/miso

#define DISP_BACKLIGHT_SWITCH 1
// #define DISP_BACKLIGHT_PWM //CONFIG_LV_DISP_BACKLIGHT_PWM

#define BACKLIGHT_ACTIVE_LVL 1  // CONFIG_LV_BACKLIGHT_ACTIVE_LVL
#define DISP_PIN_BCKL 12        // GPIO 12

#define DISP_SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 3)
#define DISP_SPI_CLOCK_SPEED_HZ (27 * 1000 * 1000)
#define DISP_SPI_MODE (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
