// Pins for ESP32S3_ZX3D50CE02S_USRC_4832
// Display driver ST7796 parallele 8080
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7796"

#define DISP_DIRECTION_LANDSCAPE (1)  // 1:landscape mode   0:portrait mode

#if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (320)
#else  // portrait mode
#define DISP_HOR_RES_MAX (320)
#define DISP_VER_RES_MAX (480)
#endif

#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * (DISP_VER_RES_MAX / 10))

#define DISP_BITS_WIDTH (8)
#define DISP_CMD_BITS_WIDTH (8)
#define DISP_PARAM_BITS_WIDTH (8)
#define DISP_CLK_FREQ (2 * 1000 * 1000)  // could be 10 if no PSRAM memory

#define DISP_BL_PIN (45)  // GPIO 45
#define DISP_CS_PIN (48)  // GPIO 48
#define DISP_RST_PIN (4)  // GPIO 4 - same as touch
#define DISP_RS_PIN (0)   // GPIO 0
#define DISP_WR_PIN (47)  // GPIO 47

#define DISP_D00_PIN (9)   // GPIO 9
#define DISP_D01_PIN (46)  // GPIO 46
#define DISP_D02_PIN (3)   // GPIO 3
#define DISP_D03_PIN (8)   // GPIO 8
#define DISP_D04_PIN (18)  // GPIO 18
#define DISP_D05_PIN (17)  // GPIO 17
#define DISP_D06_PIN (16)  // GPIO 16
#define DISP_D07_PIN (15)  // GPIO 15

#define DISP_BL_ON (1)   // Screen backlight enabled
#define DISP_BL_OFF (0)  // Screen backlight enabled

#ifdef __cplusplus
} /* extern "C" */
#endif
