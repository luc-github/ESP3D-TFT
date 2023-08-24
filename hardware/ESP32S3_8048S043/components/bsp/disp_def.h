// Pins for ESP32S3_8048S043
// Display driver RGB parallel interface
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "ST7262"

#define DISP_DIRECTION_LANDSCAPE (1)  // 1:landscape mode   0:portrait mode

#if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
#define DISP_HOR_RES_MAX (800)
#define DISP_VER_RES_MAX (480)
#else  // portrait mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (800)
#endif

#define DISP_BITS_WIDTH (16)  // RGB565 in parallel mode, thus 16bit in width

#define DISP_CLK_FREQ (13 * 1000 * 1000)  // adjusted

#define DISP_PCLK_PIN (42)
#define DISP_VSYNC_PIN (41)
#define DISP_HSYNC_PIN (39)
#define DISP_DE_PIN (40)

#define DISP_D00_PIN (8)   // B0
#define DISP_D01_PIN (3)   // B1
#define DISP_D02_PIN (46)  // B2
#define DISP_D03_PIN (9)   // B3
#define DISP_D04_PIN (1)   // B4
#define DISP_D05_PIN (5)   // G0
#define DISP_D06_PIN (6)   // G1
#define DISP_D07_PIN (7)   // G2
#define DISP_D08_PIN (15)  // G3
#define DISP_D09_PIN (16)  // G4
#define DISP_D10_PIN (4)   // G5
#define DISP_D11_PIN (45)  // R0
#define DISP_D12_PIN (48)  // R1
#define DISP_D13_PIN (47)  // R2
#define DISP_D14_PIN (21)  // R3
#define DISP_D15_PIN (14)  // R4
#define DISP_EN_PIN (-1)   // Not connected

#define DISP_HSYNC_BACK_PORSH (8)
#define DISP_HSYNC_FRONT_PORSH (8)
#define DISP_HSYNC_PULSE_WIDTH (4)
#define DISP_VSYNC_BACK_PORSH (8)
#define DISP_VSYNC_FRONT_PORSH (8)
#define DISP_VSYNC_PULSE_WIDTH (4)
#define DISP_PCLK_ACTIVE (true)
#define DISP_FB_IN_PSRAM \
  (true)  // Do not change as it is mandatory for RGB parallel interface and
          // octal PSRAM
#define DISP_NUM_FB (1)
#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER (false)

#define DISP_BACKLIGHT_SWITCH 1
#define DISP_BACKLIGHT_PWM         // CONFIG_LV_DISP_BACKLIGHT_PWM
#define BACKLIGHT_ACTIVE_LVL 1     // CONFIG_LV_BACKLIGHT_ACTIVE_LVL
#define DISP_PIN_BCKL 2            // GPIO 2
#define DISP_BCKL_DEFAULT_DUTY 20  //%

#ifdef __cplusplus
} /* extern "C" */
#endif
