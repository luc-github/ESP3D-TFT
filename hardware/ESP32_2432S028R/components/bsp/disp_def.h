// Pins for ESP32 2332S028R
// Display driver ILI9341 SPI
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ILI9341"

#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 240

// 1/10
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 10)

#define ILI9341_DC 2             // GPIO 2
#define ILI9341_USE_RST 1        // Flag
#define ILI9341_RST 4            // GPIO 4
#define ILI9341_INVERT_COLORS 0  // Flag

#define DISP_USE_DOUBLE_BUFFER 1

/*
PORTRAIT				0
PORTRAIT_INVERTED		1
LANDSCAPE				2
LANDSCAPE_INVERTED		3
*/

#define ILI9341_DISPLAY_ORIENTATION 3  // landscape
#define DISP_SPI_MISO 12               // GPIO 12
#define DISP_SPI_MOSI 13               // GPIO 13
#define DISP_SPI_CLK 14                // GPIO 14
#define DISP_SPI_CS 15                 // GPIO 15
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
#define DISP_PIN_BCKL 21        // GPIO 21
#define DISP_BCKL_DEFAULT_DUTY 100

#define DISP_SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)
#define DISP_SPI_CLOCK_SPEED_HZ (40 * 1000 * 1000)
#define DISP_SPI_MODE (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
