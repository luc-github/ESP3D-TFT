/*
  esp3d_http_service
  Copyright (c) 2022 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <math.h>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_sd.h"
#include "http/esp3d_http_service.h"

#if ESP3D_TIMESTAMP_FEATURE
#include <time.h>
#endif  // ESP3D_TIMESTAMP_FEATURE
#include "authentication/esp3d_authentication.h"

esp_err_t ESP3DHttpService::sdfiles_handler(httpd_req_t *req) {
  // Send httpd header
  httpd_resp_set_http_hdr(req);
#if ESP3D_AUTHENTICATION_FEATURE
  ESP3DAuthenticationLevel authentication_level = getAuthenticationLevel(req);
  if (authentication_level == ESP3DAuthenticationLevel::guest) {
    // send 401
    return not_authenticated_handler(req);
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Uri: %s", req->uri);
  char *buf;
  size_t buf_len;
  char param[255 + 1] = {0};
  std::string tmpstr;
  std::string path = "/";
  std::string action;
  std::string filename;
  std::string createPath;
  std::string status = "ok";
  std::string currentPath;
  if (esp3dHttpService.hasArg(req, "path")) {
    path = esp3dHttpService.getArg(req, "path");
    esp3d_log("Path from post: %s", path.c_str());
  }
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      esp3d_log("query string: %s", buf);
      if (httpd_query_key_value(buf, "path", param, 255) == ESP_OK) {
        path = esp3d_string::urlDecode(param);
        esp3d_log("path is: %s", path.c_str());
      }
      if (httpd_query_key_value(buf, "action", param, 255) == ESP_OK) {
        action = param;
        esp3d_log("action is: %s", action.c_str());
      }
      if (httpd_query_key_value(buf, "filename", param, 255) == ESP_OK) {
        filename = esp3d_string::urlDecode(param);
        esp3d_log("filename is: %s", filename.c_str());
      }
    }
    free(buf);
  }
  if (sd.accessFS()) {
    if (action.length() > 0) {
      if (filename.length() > 0) {
        // some sanity check
        std::string currentPath = path;
        if (path[path.length() - 1] != '/') {
          currentPath += "/";
        }
        if (filename[0] == '/') {
          currentPath += &(filename.c_str()[1]);
        } else {
          currentPath += filename;
        }

        if (action == "delete") {
          esp3d_log("Delete file: %s", currentPath.c_str());
          if (!sd.remove(currentPath.c_str())) {
            esp3d_log_e("Deletion failed");
            status = "delete file failed";
          }
        } else if (action == "deletedir") {
          esp3d_log("Delete dir: %s", currentPath.c_str());
          if (!sd.rmdir(currentPath.c_str())) {
            esp3d_log_e("Deletion failed");
            status = "delete dir failed";
          }
        } else if (action == "createdir") {
          esp3d_log("Create dir: %s", currentPath.c_str());
          if (!sd.mkdir(currentPath.c_str())) {
            esp3d_log_e("Creation failed");
            status = "ceate dir failed";
          }
        }
      }
    }
    uint64_t totalSpace = 0;
    uint64_t usedSpace = 0;
    sd.getSpaceInfo(&totalSpace, &usedSpace, nullptr, true);
    uint8_t occupation = 0;
    if (totalSpace == 0) {
      status = "Error getting space info";
    } else {
      occupation = round(100.0 * usedSpace / totalSpace);
      if (occupation == 0 && usedSpace != 0) {
        occupation = 1;
      }
    }

    // head of json
    if (esp3dHttpService.sendStringChunk(req, "{\"files\":[") != ESP_OK) {
      sd.releaseFS();
      return ESP_FAIL;
    }
    DIR *dir = sd.opendir(path.c_str());

    if (dir) {
      struct dirent *entry;
      struct stat entry_stat;
      uint nentries = 0;
      while ((entry = sd.readdir(dir)) != NULL) {
        currentPath = path;
        tmpstr = "";
        if (nentries > 0) {
          tmpstr += ",";
        }
        nentries++;
        if (path[path.length() - 1] != '/') {
          currentPath += "/";
        }
        currentPath += entry->d_name;
        if (entry->d_type == DT_DIR) {
          tmpstr += "{\"name\":\"";
          tmpstr += entry->d_name;
          tmpstr += "\",\"size\":\"-1\"}";

        } else {
          if (sd.stat(currentPath.c_str(), &entry_stat) == -1) {
            esp3d_log_e("Failed to stat %s : %s",
                        entry->d_type == DT_DIR ? "DIR" : "FILE",
                        currentPath.c_str());
            continue;
          }
#if ESP3D_TIMESTAMP_FEATURE
          char buff[20];
          strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S",
                   localtime(&(entry_stat.st_mtim.tv_sec)));
#endif  // ESP3D_TIMESTAMP_FEATURE
          tmpstr += "{\"name\":\"";
          tmpstr += entry->d_name;
          tmpstr += "\",\"size\":\"";
          tmpstr += esp3d_string::formatBytes(entry_stat.st_size);
#if ESP3D_TIMESTAMP_FEATURE
          tmpstr += "\",\"time\":\"";
          tmpstr += buff;
#endif  // ESP3D_TIMESTAMP_FEATURE
          tmpstr += "\"}";
        }
        if (esp3dHttpService.sendStringChunk(req, tmpstr.c_str()) != ESP_OK) {
          sd.releaseFS();
          return ESP_FAIL;
        }
      }
    } else {
      status = "error cannot access ";
      status += path;
    }

    tmpstr = "],\"path\":\"";
    tmpstr += path;
    tmpstr += "\",\"occupation\":\"";
    tmpstr += std::to_string(occupation);
    tmpstr += "\",\"status\":\"";
    tmpstr += status;
    tmpstr += "\",\"total\":\"";
    tmpstr += esp3d_string::formatBytes(totalSpace);
    tmpstr += "\",\"used\":\"";
    tmpstr += esp3d_string::formatBytes(usedSpace);
    tmpstr += "\"}";
    if (esp3dHttpService.sendStringChunk(req, tmpstr.c_str()) != ESP_OK) {
      sd.releaseFS();
      return ESP_FAIL;
    }
    // end of json
    httpd_resp_send_chunk(req, NULL, 0);
    sd.releaseFS();
    return ESP_OK;
  } else {
    httpd_resp_sendstr(req, "{\"status\":\"error accessing filesystem\"}");
    return ESP_FAIL;
  }
}
