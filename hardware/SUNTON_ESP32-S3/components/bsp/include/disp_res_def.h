// Display resolution definitions for: ESP32_8048S070C, ESP32_8048S050C, ESP32_8048S043C, ESP32_4827S043C
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_ESP32S3_8048S070C || TARGET_ESP32S3_8048S050C || TARGET_ESP32S3_8048S043C
  #define TFT_WIDTH     800
  #define TFT_HEIGHT    480
#elif TARGET_ESP32S3_4827S043C
  #define TFT_WIDTH     480
  #define TFT_HEIGHT    272
#endif

/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/

#define DISP_ORIENTATION 2  // landscape

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
