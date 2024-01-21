/*
  camera.cpp -  camera functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if ESP3D_CAMERA_FEATURE
#include "camera.h"
#include "camera_def.h"
#include "esp3d_log.h"

#include <sys/param.h>
#include <esp_system.h>
#include <string.h>
#include <esp_camera.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "http/esp3d_http_service.h"

#define DEFAULT_FRAME_SIZE FRAMESIZE_HVGA
#define JPEG_COMPRESSION 80

Camera esp3d_camera;

bool Camera::handle_snap(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp3d_log("Camera stream reached");
    if (!_initialised)
    {
        esp3d_log("Camera not started");
        if (req)
        {
            httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
            httpd_resp_set_type(req, "text/plain");
            httpd_resp_sendstr(req, "Camera not started");
        }
        return false;
    }
    esp3d_log("Camera capture ongoing");
    fb = esp_camera_fb_get();
    if (!fb)
    {
        esp3d_log("Camera capture failed");
        httpd_resp_send_500(req);
        return false;
    }
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return true;
}
Camera::Camera()
{
    _started = false;
    _initialised = false;
}

Camera::~Camera()
{
    end();
}

int Camera::command(const char *param, const char *value)
{
    esp3d_log("Camera: %s=%s\n", param, value);
    int res = 0;
    int val = atoi(value);
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
    {
        res = -1;
    }
#if CAM_LED_PIN != -1
    if (!strcmp(param, "light"))
    {
        digitalWrite(CAM_LED_PIN, val == 1 ? HIGH : LOW);
    }
    else
#endif // CAM_LED_PIN
        if (!strcmp(param, "framesize"))
        {
            if (s->pixformat == PIXFORMAT_JPEG)
            {
                res = s->set_framesize(s, (framesize_t)val);
            }
        }
        else if (!strcmp(param, "quality"))
        {
            res = s->set_quality(s, val);
        }
        else if (!strcmp(param, "contrast"))
        {
            res = s->set_contrast(s, val);
        }
        else if (!strcmp(param, "brightness"))
        {
            res = s->set_brightness(s, val);
        }
        else if (!strcmp(param, "saturation"))
        {
            res = s->set_saturation(s, val);
        }
        else if (!strcmp(param, "gainceiling"))
        {
            res = s->set_gainceiling(s, (gainceiling_t)val);
        }
        else if (!strcmp(param, "colorbar"))
        {
            res = s->set_colorbar(s, val);
        }
        else if (!strcmp(param, "awb"))
        {
            res = s->set_whitebal(s, val);
        }
        else if (!strcmp(param, "agc"))
        {
            res = s->set_gain_ctrl(s, val);
        }
        else if (!strcmp(param, "aec"))
        {
            res = s->set_exposure_ctrl(s, val);
        }
        else if (!strcmp(param, "hmirror"))
        {
            res = s->set_hmirror(s, val);
        }
        else if (!strcmp(param, "vflip"))
        {
            res = s->set_vflip(s, val);
        }
        else if (!strcmp(param, "awb_gain"))
        {
            res = s->set_awb_gain(s, val);
        }
        else if (!strcmp(param, "agc_gain"))
        {
            res = s->set_agc_gain(s, val);
        }
        else if (!strcmp(param, "aec_value"))
        {
            res = s->set_aec_value(s, val);
        }
        else if (!strcmp(param, "aec2"))
        {
            res = s->set_aec2(s, val);
        }
        else if (!strcmp(param, "dcw"))
        {
            res = s->set_dcw(s, val);
        }
        else if (!strcmp(param, "bpc"))
        {
            res = s->set_bpc(s, val);
        }
        else if (!strcmp(param, "wpc"))
        {
            res = s->set_wpc(s, val);
        }
        else if (!strcmp(param, "raw_gma"))
        {
            res = s->set_raw_gma(s, val);
        }
        else if (!strcmp(param, "lenc"))
        {
            res = s->set_lenc(s, val);
        }
        else if (!strcmp(param, "special_effect"))
        {
            res = s->set_special_effect(s, val);
        }
        else if (!strcmp(param, "wb_mode"))
        {
            res = s->set_wb_mode(s, val);
        }
        else if (!strcmp(param, "ae_level"))
        {
            res = s->set_ae_level(s, val);
        }
        else
        {
            res = -1;
        }
    return res;
}

bool Camera::initHardware()
{
    _initialised = false;
    esp3d_log("Disable brown out");
    // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
    stopHardware();
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d7 = CAM_PIN_D7;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_pclk = CAM_PIN_PCLK;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000; // 这个值必须是一个能被 80 MHz 整数分频的频率,
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = 12; // 0-63 lower number means higher quality
    config.fb_count = 1;      // if more than one, i2s runs in continuous mode. Use only with JPEG
                              // 控制使用 framebuffer 的个数,个数越多消耗内存越大。其值大于 2 时,获取的一帧图像可能不是实时的
    config.frame_size = DEFAULT_FRAME_SIZE;
    config.fb_location = CAMERA_FB_IN_PSRAM; //
    config.grab_mode = CAMERA_GRAB_LATEST;   // ;    CAMERA_GRAB_LATEST
    esp3d_log("Init camera");
#if CAM_PULLUP1 != -1
    pinMode(CAM_PULLUP1, INPUT_PULLUP);
#endif // CAM_PULLUP1
#if CAM_PULLUP2 != -1
    pinMode(CAM_PULLUP2, INPUT_PULLUP);
#endif // CAM_PULLUP2
#if CAM_LED_PIN != -1
    pinMode(CAM_LED_PIN, OUTPUT);
    digitalWrite(CAM_LED_PIN, LOW);
#endif // CAM_LED_PIN
       // initialize the camera
    esp3d_log("Init camera config");
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        esp3d_log_e("Camera init failed with error 0x%x", err);
    }
    else
    {
        _initialised = true;
    }
    return _initialised;
}

bool Camera::stopHardware()
{
    return true;
}

// need to be call by device and by network
bool Camera::begin()
{
    esp3d_log("init camera");
    initHardware();
    end();
    esp3d_log("Begin camera");
    if (!_initialised)
    {
        esp3d_log("Init hardware not done");
        return false;
    }
    esp3d_log("Init camera sensor settings");
    sensor_t *s = esp_camera_sensor_get();
    if (s != nullptr)
    {
        // initial sensors are flipped vertically and colors are a bit saturated
        if (s->id.PID == OV3660_PID)
        {
            s->set_brightness(s, 1);  // up the blightness just a bit
            s->set_saturation(s, -2); // lower the saturation
        }

        s->set_framesize(s, DEFAULT_FRAME_SIZE);

#if defined(CAMERA_DEVICE_FLIP_HORIZONTALY)
        s->set_hmirror(s, 1);
#endif // CAMERA_DEVICE_FLIP_HORIZONTALY
#if defined(CAMERA_DEVICE_FLIP_VERTICALY)
        s->set_vflip(s, 1);
#endif // CAMERA_DEVICE_FLIP_VERTICALY
    }
    else
    {
        esp3d_log("Cannot access camera sensor");
    }
    _started = _initialised;
    return _started;
}

void Camera::end()
{
    _started = false;
}

void Camera::handle()
{
    // nothing to do
}
uint8_t Camera::GetModel()
{
    return 0;
}
const char *Camera::GetModelString()
{
    return "ESP32 Cam";
}
#endif // CAMERA_DEVICE
