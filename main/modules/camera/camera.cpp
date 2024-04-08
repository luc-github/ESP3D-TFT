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
#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

Camera esp3d_camera;

bool Camera::handle_snap(httpd_req_t *req, const char *path,
                         const char *filename) {
  camera_fb_t *fb = NULL;
  bool has_error = false;
  esp3d_log("Camera stream reached");
  if (!req && !path && !filename) {
    esp3d_log_e("Invalid parameters");
    return false;
  }
  if (!_started) {
    esp3d_log_e("Camera not started");
    if (req) {
      esp3dHttpService.httpd_resp_set_http_hdr(req);
      httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
      httpd_resp_set_type(req, "text/plain");
      httpd_resp_sendstr(req, "Camera not started");
    }
    return false;
  }
  // check if parameters are present
  if (req) {
    size_t buf_len;
    char *buf = NULL;
    char param[255 + 1] = {0};
    std::string tmpstr;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) {
      esp3d_log_e("Cannot access camera sensor");
      return false;
    }
    if (buf_len > 1) {
      buf = (char *)malloc(buf_len);
      if (buf) {
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
          esp3d_log("query string: %s", buf);
          // check framesize
          if (httpd_query_key_value(buf, "framesize", param, 255) == ESP_OK) {
            tmpstr = esp3d_string::urlDecode(param);
            esp3d_log("framesize is: %s", tmpstr.c_str());
            // set framesize
            command("framesize", tmpstr.c_str());
          }
          // hmirror
          if (httpd_query_key_value(buf, "hmirror", param, 255) == ESP_OK) {
            tmpstr = esp3d_string::urlDecode(param);
            esp3d_log("hmirror is: %s", tmpstr.c_str());
            // set hmirror
            command("hmirror", tmpstr.c_str());
          }
          // vflip
          if (httpd_query_key_value(buf, "vflip", param, 255) == ESP_OK) {
            tmpstr = esp3d_string::urlDecode(param);
            esp3d_log("vflip is: %s", tmpstr.c_str());
            // set vflip
            command("vflip", tmpstr.c_str());
          }
          // wb_mode
          if (httpd_query_key_value(buf, "wb_mode", param, 255) == ESP_OK) {
            tmpstr = esp3d_string::urlDecode(param);
            esp3d_log("wb_mode is: %s", tmpstr.c_str());
            // set wb_mode
            command("wb_mode", tmpstr.c_str());
          }
        }
        free(buf);
      } else {
        esp3d_log_e("Memory allocation failed");
        return false;
      }
    }
  } else {
#if ESP3D_SD_CARD_FEATURE
    if (!path || !filename) {
      esp3d_log_e("Invalid parameters");
      return false;
    }
#else
    esp3d_log_e("Invalid parameters");
    return false;
#endif  // ESP3D_SD_CARD_FEATURE
  }
  esp3d_log("Camera capture ongoing");
  fb = esp_camera_fb_get();
  if (!fb) {
    esp3d_log("Camera capture failed");
    if (req) {
      esp3dHttpService.httpd_resp_set_http_hdr(req);
      httpd_resp_send_500(req);
    }
    return false;
  }
  if (req) {
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition",
                       "inline; filename=capture.jpg");
    httpd_resp_send(req, (const char *)fb->buf, fb->len);
  } else {
#if ESP3D_SD_CARD_FEATURE
    std::string fullpath = path;
    if (fullpath.back() != '/') fullpath += "/";
    fullpath += filename;
    if (sd.accessFS()) {
      esp3d_log("Will Save snap to %s", fullpath.c_str());
      if (!sd.exists(path)) {
        esp3d_log("Path does not exist, creating it");
        if (!sd.mkdir(path)) {
          has_error = true;
          esp3d_log_e("Create path failed");
        }
      }
      if (!has_error) {
        // TODO: Need to check space availability ?
        FILE *fd = sd.open(fullpath.c_str(), "w");
        if (fd) {
          if (fwrite(fb->buf, fb->len, 1, fd) != 1) {
            esp3d_log_e("Write file failed");
            has_error = true;
          } else {
            esp3d_log("Snap saved to %s", fullpath.c_str());
          }
          sd.close(fd);
        } else {
          esp3d_log_e("Create file failed");
          has_error = true;
        }
      }
      sd.releaseFS();
    } else {
      esp3d_log_e("Cannot access SD card");
      has_error = true;
    }
#endif  // ESP3D_SD_CARD_FEATUREm
  }
  // free memory
  if (fb) esp_camera_fb_return(fb);
  return !has_error;
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
