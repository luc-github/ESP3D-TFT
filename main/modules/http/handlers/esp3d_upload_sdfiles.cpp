/*
  esp3d_http_service : upload_to_sd_handler
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

#if ESP3D_SD_CARD_FEATURE
#include <sys/param.h>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "filesystem/esp3d_sd.h"
#include "http/esp3d_http_service.h"

esp_err_t Esp3dHttpService::upload_to_sd_handler(
    const uint8_t* data, size_t datasize, Esp3dUploadState file_upload_state,
    const char* filename, size_t filesize) {
  // No need Authentication as already handled in multipart_parser
  static FILE* FileFD = nullptr;
  static bool isAccessed = false;
  switch (file_upload_state) {
    case Esp3dUploadState::upload_start:
      esp3d_log("Starting sd upload:%s", filename);
      if (FileFD) {
        sd.close(FileFD);
        FileFD = nullptr;
      }
      if (!sd.accessFS()) {
        esp3d_log_e("Error accessing sd filesystem");
        esp3dHttpService.pushError(Esp3dUploadError::access_denied,
                                   "Error accessing sd filesystem");
        return ESP_FAIL;
      }
      isAccessed = true;
      if (filesize != (size_t)-1) {
        uint64_t freespace = 0;
        sd.getSpaceInfo(nullptr, nullptr, &freespace);
        if (freespace < filesize) {
          esp3d_log_e(
              "Error not enough space on sd filesystem have %lld and need %d",
              freespace, filesize);
          esp3dHttpService.pushError(Esp3dUploadError::not_enough_space,
                                     "Error not enough space");
          return ESP_FAIL;
        }
      }
      FileFD = sd.open(filename, "w");
      if (!FileFD) {
        esp3d_log_e("Error cannot create %s on sd filesystem", filename);
        esp3dHttpService.pushError(Esp3dUploadError::file_create_failed,
                                   "Error file creation failed");
        return ESP_FAIL;
      }
      break;
    case Esp3dUploadState::file_write:
      // esp3d_log("Write :%d bytes", datasize);
      if (datasize && FileFD) {
        if (fwrite(data, datasize, 1, FileFD) != 1) {
          esp3d_log_e("Error cannot writing data on sd filesystem ");
          esp3dHttpService.pushError(Esp3dUploadError::write_failed,
                                     "Error file write failed");
          return ESP_FAIL;
        }
      }
      break;
    case Esp3dUploadState::upload_end:
      esp3d_log("Ending upload");
      sd.close(FileFD);
      FileFD = nullptr;
      if (filesize != (size_t)-1) {
        struct stat entry_stat;
        if (sd.stat(filename, &entry_stat) == -1 ||
            entry_stat.st_size != filesize) {
          if (entry_stat.st_size != datasize) {
            esp3d_log_e("Invalid size got %d expected %d ",
                        (size_t)entry_stat.st_size, filesize);
          } else {
            esp3d_log_e("Failed to stat %s", filename);
          }
          sd.remove(filename);
          sd.releaseFS();
          esp3dHttpService.pushError(
              Esp3dUploadError::wrong_size,
              "Error file size does not match expected one");
          return ESP_FAIL;
        }
      }
      isAccessed = false;
      sd.releaseFS();
      break;
    case Esp3dUploadState::upload_aborted:
      esp3d_log("Error happened: cleanup");
      if (FileFD) {
        sd.close(FileFD);
      }
      FileFD = nullptr;
      if (isAccessed) {
        sd.remove(filename);
        sd.releaseFS();
      }
      isAccessed = false;
      break;
  }
  return ESP_OK;
}

#endif  // ESP3D_SD_CARD_FEATURE