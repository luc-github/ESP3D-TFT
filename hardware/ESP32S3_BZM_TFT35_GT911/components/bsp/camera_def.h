// Pins definition for ESP32 ESP32S3_BZM_TFT35_GT911
// CAMERA OV2640
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief ESP32S3_BZM_TFT35_GT911 CAMERA OV2640 GPIO definition
     *
     */

// PIN Map
#define CAM_LED_PIN -1
#define CAM_PULLUP1 -1
#define CAM_PULLUP2 -1

#define CAM_PIN_PWDN -1  // power down is not used
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 8
#define CAM_PIN_SIOD 47
#define CAM_PIN_SIOC 21

#define CAM_PIN_D7 3
#define CAM_PIN_D6 18
#define CAM_PIN_D5 17
#define CAM_PIN_D4 15
#define CAM_PIN_D3 6
#define CAM_PIN_D2 5
#define CAM_PIN_D1 4
#define CAM_PIN_D0 7
#define CAM_PIN_VSYNC 9
#define CAM_PIN_HREF 46
#define CAM_PIN_PCLK 16

#ifdef __cplusplus
} /* extern "C" */
#endif
