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

#include <stdio.h>
#include <sys/utime.h>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_vfs.h"
#include "filesystem/esp3d_globalfs.h"
#include "http/esp3d_http_service.h"
#include "time.h"

esp_err_t ESP3DHttpService::webdav_get_handler(httpd_req_t* req) {
  esp3d_log_d("Uri: %s", req->uri);
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log_d("Uri: %s", uri.c_str());
  esp3d_log_d("Path: %s", esp3d_string::getPathFromString(uri.c_str()));
  esp3d_log_d("filename: %s", esp3d_string::getFilenameFromString(uri.c_str()));

  // Tue, 10 Jan 2023 09:00:00GMT
  //"%a, %d %b %Y %H:%M:%S GMT"

  // http://www.webdav.org/specs/rfc2518.xml#rfc.section.8.1.2
  // 1997-12-01T17:42:21-08:00
  //"%Y-%m-%dT%H:%M:%S%z"

  /*

  if (sd.stat(currentPath.c_str(), &entry_stat) == -1) {
            esp3d_log_e("Failed to stat");
          }
  char buff[20];
          strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S",
                   gmtime(&(entry_stat.st_mtim.tv_sec)));
  */
  if (globalFs.accessFS(uri.c_str())) {
    struct stat entry_stat;
    char buff[40];
    if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
      esp3d_log_e("Failed to stat");
    } else {
      // st_ctime = creation
      // st_atime = last access
      // st_mtime = last modification
      size_t res = strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT",
                            gmtime(&(entry_stat.st_mtim.tv_sec)));
      if (res == 0) {
        esp3d_log_e("Failed to convert date");
      } else {
        esp3d_log_d("Date: %s", buff);
      }
    }
    globalFs.releaseFS(uri.c_str());
  } else {
    esp3d_log_e("Failed to access FS");
  }

  /*
   struct tm tm;
   // original:  "2023-02-15T16:27:36-0500"
   std::string offset_str = "-0500";
   std::string date_string = "2023-02-15T16:27:36";
   memset(&tm, 0, sizeof(struct tm));
   int offset_h = atoi(offset_str.substr(0, 3).c_str());  // 5
   int offset_m = atoi(offset_str.substr(3, 2).c_str());  // 0
   int offset_s = offset_h * 3600 + offset_m * 60;        // -18000
   esp3d_log_d("offset_s: %d", offset_s);
   if (strptime(date_string.c_str(), "%Y-%m-%dT%H:%M:%S", &tm) != NULL) {
     time_t new_timestamp = mktime(&tm);
     new_timestamp += offset_s;
     struct utimbuf timestamp;
     timestamp.actime = timestamp.modtime = new_timestamp;
     esp_vfs_utime(uri.c_str(), &timestamp);
   } else {
     esp3d_log_e("Failed to convert date");
   }


//set time
 struct tm tm;
 time_t seconds;

 if (strptime("2023-10-10T11:08:47.000Z", "%Y-%m-%dT%H:%M:%S.000Z", &tm) !=
     NULL) {
   seconds = mktime(&tm);

   struct timeval tv;
   tv.tv_sec = seconds;
   tv.tv_usec = 0;

   settimeofday(&tv, NULL);
 } else {
   esp3d_log_e("Failed to convert date");
 }

 time_t rawtime;

 time(&rawtime);

 esp3d_log_d("%s", ctime(&rawtime));*/

  // TODO: implement method GET
  // extract path from uri
  // clear payload from request if any
  // Check can access (error code 503)
  // Check if file exist(error code 404)
  // Check if file is a directory
  // if file send content-type and content-length header
  // if directory, send 200 response and return
  // read file and send it (error code 500 if any error)
  // close file
  // release access
  // response code 200 if success

  return webdav_send_response(req, 200, "");
}
