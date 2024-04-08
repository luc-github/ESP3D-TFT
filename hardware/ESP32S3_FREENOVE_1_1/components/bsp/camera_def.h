// Pins definition for FREENOVE Camera
// CAMERA ESP32S3_EYE
#pragma once
#include "driver/gpio.h"
#include "esp32_camera.h"
#include "sdkconfig.h"
#ifdef __cplusplus
extern "C" {
#endif
// Camera configuration
// it is a OV2640 camera module
const esp32_camera_config_t camera_config = {
    .hw_config =
        {
            .pin_pwdn = -1,     /*!< GPIO pin for camera power down line */
            .pin_reset = -1,    /*!< GPIO pin for camera reset line */
            .pin_xclk = 1,      /*!< GPIO pin for camera XCLK line */
            .pin_sccb_sda = 4, /*!< GPIO pin for camera SDA line */
            .pin_sccb_scl = 5, /*!< GPIO pin for camera SCL line */
            .pin_d7 = 16,        /*!< GPIO pin for camera D7 line */
            .pin_d6 = 17,       /*!< GPIO pin for camera D6 line */
            .pin_d5 = 18,       /*!< GPIO pin for camera D5 line */
            .pin_d4 = 12,       /*!< GPIO pin for camera D4 line */
            .pin_d3 = 10,        /*!< GPIO pin for camera D3 line */
            .pin_d2 = 8,        /*!< GPIO pin for camera D2 line */
            .pin_d1 = 9,        /*!< GPIO pin for camera D1 line */
            .pin_d0 = 11,        /*!< GPIO pin for camera D0 line */
            .pin_vsync = 6,     /*!< GPIO pin for camera VSYNC line */
            .pin_href = 7,     /*!< GPIO pin for camera HREF line */
            .pin_pclk = 13,     /*!< GPIO pin for camera PCLK line */

            .xclk_freq_hz =
                20 * 1000 *
                1000, /*!< Frequency of XCLK signal, in Hz. EXPERIMENTAL: Set to
                         16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode */

            .ledc_timer =
                LEDC_TIMER_0, /*!< LEDC timer to be used for generating XCLK  */
            .ledc_channel = LEDC_CHANNEL_0, /*!< LEDC channel to be used for
                                               generating XCLK  */

            .pixel_format =
                PIXFORMAT_JPEG, /*!< Format of the pixel data: PIXFORMAT_ +
                                   YUV422|GRAYSCALE|RGB565|JPEG  */
            .frame_size =
                FRAMESIZE_VGA, /*!< Size of the output image: FRAMESIZE_ +
                                  QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA  */

            .jpeg_quality = 12, /*!< Quality of JPEG output. 0-63 lower means
                                   higher quality  */
            .fb_count =
                1, /*!< Number of frame buffers to be allocated. If more than
                      one, then each frame will be acquired (double speed)  */
            .fb_location = CAMERA_FB_IN_PSRAM, /*!< The location where the frame
                                                  buffer will be allocated */
            .grab_mode =
                CAMERA_GRAB_WHEN_EMPTY, /*!< When buffers should be filled */
#if CONFIG_CAMERA_CONVERTER_ENABLED
            .conv_mode = 0, /*!< RGB<->YUV Conversion mode */
#endif

            .sccb_i2c_port = 0, /*!< If pin_sccb_sda is -1, use the already
                                   configured I2C bus by number */
        },
    .pin_pullup_1 = -1, /* if any need */
    .pin_pullup_2 = -1, /* if any need */
    .pin_led = -1,
    .flip_horizontaly = false, // if horizontal flip is needed
    .flip_vertically = false, // if vertical flip is needed
    .brightness = 0, // default value is 0
    .contrast = 0,  // default value is 0
    .name = "Freenove (ESP32S3 Eye)",
} ;

#ifdef __cplusplus
} /* extern "C" */
#endif
