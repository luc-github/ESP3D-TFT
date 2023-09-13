// Display Resolution definitions for: ESP32_3248S035C, ESP32_3248S035R, ESP32_2432S028R
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_ESP32_3248S035C || TARGET_ESP32_3248S035R
  #define TFT_WIDTH     480
  #define TFT_HEIGHT    320
#elif TARGET_ESP32_2432S028R
  #define TFT_WIDTH     320
  #define TFT_HEIGHT    240
#endif

/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/

#define DISP_ORIENTATION 3  // landscape inverted

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
  #define DISP_LANDSCAPE  1

  #define DISP_HOR_RES_MAX  TFT_WIDTH
  #define DISP_VER_RES_MAX  TFT_HEIGHT
#else  // portrait mode
  #define DISP_PORTRAIT   1

  #define DISP_HOR_RES_MAX  TFT_HEIGHT
  #define DISP_VER_RES_MAX  TFT_WIDTH
#endif

#if DISP_ORIENTATION == 1 || DISP_ORIENTATION == 3  // mirrored
  #define DISP_MIRRORED   1
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
