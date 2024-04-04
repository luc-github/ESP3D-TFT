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

#if defined(ESP3D_CAMERA_FEATURE) && ESP3D_CAMERA_FEATURE == 1
#include "camera.h"

#include <esp_system.h>
#include <string.h>
#include <sys/param.h>

#include "esp32_camera.h"
#include "esp3d_log.h"
#include "http/esp3d_http_service.h"

Camera esp3d_camera;

bool Camera::handle_snap(httpd_req_t *req, const char *path,
                         const char *filename) {
  camera_fb_t *fb = NULL;
  esp3d_log("Camera stream reached");
  if (!_started) {
    esp3d_log("Camera not started");
    if (req) {
      httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
      httpd_resp_set_type(req, "text/plain");
      httpd_resp_sendstr(req, "Camera not started");
    }
    return false;
  }
  esp3d_log("Camera capture ongoing");
  fb = esp_camera_fb_get();
  if (!fb) {
    esp3d_log("Camera capture failed");
    if (req) httpd_resp_send_500(req);
    return false;
  }
  if (req) {
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition",
                       "inline; filename=capture.jpg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);
  }
  esp_camera_fb_return(fb);
  return true;
}
Camera::Camera() { _started = false; }

Camera::~Camera() { end(); }

int Camera::command(const char *param, const char *value) {
  esp3d_log("Camera: %s=%s\n", param, value);
  int res = 0;
  int val = atoi(value);
  sensor_t *s = esp_camera_sensor_get();
  if (s == nullptr) {
    res = -1;
  }

  if (!strcmp(param, "light")) {
    if (esp32_camera_power_led(val) == ESP_OK) {
      res = 0;
    } else {
      res = -1;
    }
  } else if (!strcmp(param, "framesize")) {
    if (s->pixformat == PIXFORMAT_JPEG) {
      res = s->set_framesize(s, (framesize_t)val);
    }
  } else if (!strcmp(param, "quality")) {
    res = s->set_quality(s, val);
  } else if (!strcmp(param, "contrast")) {
    res = s->set_contrast(s, val);
  } else if (!strcmp(param, "brightness")) {
    res = s->set_brightness(s, val);
  } else if (!strcmp(param, "saturation")) {
    res = s->set_saturation(s, val);
  } else if (!strcmp(param, "gainceiling")) {
    res = s->set_gainceiling(s, (gainceiling_t)val);
  } else if (!strcmp(param, "colorbar")) {
    res = s->set_colorbar(s, val);
  } else if (!strcmp(param, "awb")) {
    res = s->set_whitebal(s, val);
  } else if (!strcmp(param, "agc")) {
    res = s->set_gain_ctrl(s, val);
  } else if (!strcmp(param, "aec")) {
    res = s->set_exposure_ctrl(s, val);
  } else if (!strcmp(param, "hmirror")) {
    res = s->set_hmirror(s, val);
  } else if (!strcmp(param, "vflip")) {
    res = s->set_vflip(s, val);
  } else if (!strcmp(param, "awb_gain")) {
    res = s->set_awb_gain(s, val);
  } else if (!strcmp(param, "agc_gain")) {
    res = s->set_agc_gain(s, val);
  } else if (!strcmp(param, "aec_value")) {
    res = s->set_aec_value(s, val);
  } else if (!strcmp(param, "aec2")) {
    res = s->set_aec2(s, val);
  } else if (!strcmp(param, "dcw")) {
    res = s->set_dcw(s, val);
  } else if (!strcmp(param, "bpc")) {
    res = s->set_bpc(s, val);
  } else if (!strcmp(param, "wpc")) {
    res = s->set_wpc(s, val);
  } else if (!strcmp(param, "raw_gma")) {
    res = s->set_raw_gma(s, val);
  } else if (!strcmp(param, "lenc")) {
    res = s->set_lenc(s, val);
  } else if (!strcmp(param, "special_effect")) {
    res = s->set_special_effect(s, val);
  } else if (!strcmp(param, "wb_mode")) {
    res = s->set_wb_mode(s, val);
  } else if (!strcmp(param, "ae_level")) {
    res = s->set_ae_level(s, val);
  } else {
    res = -1;
  }
  return res;
}

// need to be call by device and by network
bool Camera::begin() {
  esp3d_log("init camera");

  end();
  esp3d_log("Begin camera");

  sensor_t *s = esp_camera_sensor_get();
  if (s == nullptr) {
    esp3d_log("Cannot access camera sensor");
    return false;
  }
  _started = true;
  return _started;
}

void Camera::end() { _started = false; }

void Camera::handle() {
  // nothing to do
}
uint8_t Camera::GetModel() { return 0; }
const char *Camera::GetModelString() { return esp32_camera_get_name(); }
#endif  // CAMERA_DEVICE
