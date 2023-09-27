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

#include <sys/param.h>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"
#include "network/esp3d_network.h"

#if ESP3D_TFT_BENCHMARK
#include "esp_timer.h"
#endif  // ESP3D_TFT_BENCHMARK

enum class ESP3DParseState : uint8_t {
  boundary,
  content_disposition,
  content_type,
  content_separator,
  data_form,
  data_file
};

esp_err_t ESP3DHttpService::post_multipart_handler(httpd_req_t *req) {
  esp3d_log("Post Data %d on : %s", req->content_len, req->uri);

#if ESP3D_AUTHENTICATION_FEATURE
  if (strncasecmp(req->uri, "/login", 6) != 0) {
    esp3d_log("Not a login request: %s", req->uri);
    ESP3DAuthenticationLevel authentication_level = getAuthenticationLevel(req);
    if (authentication_level == ESP3DAuthenticationLevel::guest) {
      // send 401
      return not_authenticated_handler(req);
    }
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_TFT_BENCHMARK
  uint64_t startBenchmark = esp_timer_get_time();
#endif  // ESP3D_TFT_BENCHMARK
  bool hasError = false;
  char *packet = nullptr;
  char *packetWrite = nullptr;
  const char *boundaryPtr = nullptr;
  PostUploadContext *post_upload_ctx = (PostUploadContext *)req->user_ctx;
  if (!post_upload_ctx) {
    esp3d_log_e("Context not found");
    esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
    hasError = true;
  }
  if (!hasError) {
    if (!(post_upload_ctx->nextHandler) || !(post_upload_ctx->packetReadSize)) {
      esp3d_log_e("Post parameter not found");
      esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
      hasError = true;
    }
    if (!hasError) {
      boundaryPtr = esp3dHttpService.getBoundaryString(req);
      if (!boundaryPtr) {
        esp3d_log_e("No boundary found");
        esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
        hasError = true;
      }
    }
    if (!hasError) {
      packet = (char *)malloc(post_upload_ctx->packetReadSize);
      if (!packet) {
        esp3d_log_e("Memory issue, allocation failed");
        esp3dHttpService.pushError(ESP3DUploadError::memory_allocation,
                                   "Upload error");
        hasError = true;
      }
    }
    if (!hasError && post_upload_ctx->writeFn &&
        post_upload_ctx->packetWriteSize) {
      packetWrite = (char *)malloc(post_upload_ctx->packetWriteSize);
      if (!packetWrite) {
        esp3d_log_e("Memory issue, allocation failed");
        esp3dHttpService.pushError(ESP3DUploadError::memory_allocation,
                                   "Upload error");
        hasError = true;
      }
    }
  }
  // be sure list is empty
  post_upload_ctx->args.clear();
  uint8_t boundaryCursor = 2;

  uint indexPacketWrite = 0;
  std::string boundaryString = "\r\n--";
  std::string contentBuffer;
  char prevChar = 0x0;
  std::string argName = "";
  std::string argValue = "";
  std::string fileName = "";
  ESP3DUploadState uploadState = ESP3DUploadState::upload_start;
  boundaryString += boundaryPtr;
  boundaryString += "\r\n";
  esp3d_log("Boundary is: %s", boundaryString.c_str());

  size_t remaining = req->content_len;
  int32_t notProcessed = req->content_len;
  int32_t received;
  int fileSize = -1;
  // processing the content
  if (boundaryPtr && !hasError) {
    ESP3DParseState parsing_state = ESP3DParseState::boundary;
    while (remaining > 0 && !hasError) {
      if ((received = httpd_req_recv(req, packet,
                                     post_upload_ctx->packetReadSize)) <= 0) {
        esp3d_log_e("Connection lost");
        uploadState = ESP3DUploadState::upload_aborted;
        esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
        hasError = true;
      }
      if (received == HTTPD_SOCK_ERR_TIMEOUT) {
        esp3d_log_e("Time out");
        continue;
      }
      if (received == HTTPD_SOCK_ERR_INVALID ||
          received == HTTPD_SOCK_ERR_FAIL) {
        esp3d_log_e("Error connection");
        uploadState = ESP3DUploadState::upload_aborted;
        esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
        hasError = true;
      }
      // decrease received bytes from remaining bytes amount
      remaining -= received;
      // esp3d_log("received %ld bytes now total left expected %d",received,
      // remaining); Parsing the buffer
      for (uint pIndex = 0; pIndex < received && !hasError; pIndex++) {
        // esp3d_log("Processing %c %d", packet[pIndex],packet[pIndex]);
        notProcessed--;
        // esp3d_log("left %ld / %d", notProcessed, req->content_len);
        //  parse buffer
        switch (parsing_state) {
          //--XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
          case ESP3DParseState::boundary:
            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (boundaryCursor < boundaryString.length()) {
              if (boundaryString[boundaryCursor] == packet[pIndex]) {
                boundaryCursor++;
                if (boundaryCursor == boundaryString.length()) {
                  parsing_state = ESP3DParseState::content_disposition;
                }
              } else {
                if (notProcessed > 4) {
                  esp3d_log_e("Error parsing boundary %ld, %c %d", notProcessed,
                              packet[pIndex], packet[pIndex]);
                  esp3dHttpService.pushError(ESP3DUploadError::upload,
                                             "Upload error");
                  hasError = true;
                } else {
                  if (packet[pIndex] == '-' || packet[pIndex] == '\r' ||
                      packet[pIndex] == '\n') {
                    // esp3d_log("End of boundary body");
                  } else {
                    esp3d_log_e("Error parsing end boundary remaining: %d",
                                remaining);
                    esp3dHttpService.pushError(ESP3DUploadError::upload,
                                               "Upload error");
                    hasError = true;
                  }
                }
              }
            } else {
              esp3d_log_e("Error parsing end of boundary");
              esp3dHttpService.pushError(ESP3DUploadError::upload,
                                         "Upload error");
              hasError = true;
            }
            break;
          case ESP3DParseState::content_disposition:
            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (packet[pIndex] == '\n') {
              if (prevChar == '\r') {
                esp3d_log("Got %s", contentBuffer.c_str());
                if (!esp3d_string::startsWith(
                        contentBuffer.c_str(),
                        "Content-Disposition: form-data; ")) {
                  esp3d_log_e(
                      "Error parsing content disposition,missing "
                      "content-disposition header");
                  esp3dHttpService.pushError(ESP3DUploadError::upload,
                                             "Upload error");
                  hasError = true;
                }
                // check name parameter
                int startPos =
                    esp3d_string::find(contentBuffer.c_str(), "name=");
                if (startPos == -1) {
                  esp3d_log_e(
                      "Error parsing content disposition,missing name "
                      "parameter");
                  esp3dHttpService.pushError(ESP3DUploadError::upload,
                                             "Upload error");
                  hasError = true;
                }
                startPos += 6;  // size of name="
                int endPos =
                    esp3d_string::find(contentBuffer.c_str(), "\"", startPos);
                if (endPos == -1) {
                  esp3d_log_e(
                      "Error parsing content disposition,missing name "
                      "parameter");
                  esp3dHttpService.pushError(ESP3DUploadError::upload,
                                             "Upload error");
                  hasError = true;
                }
                argName = contentBuffer.substr(startPos, endPos - startPos);
                esp3d_log("Got name=%s", argName.c_str());

                // check fileName parameter
                startPos =
                    esp3d_string::find(contentBuffer.c_str(), "filename=");
                if (startPos != -1) {
                  startPos += 10;  // size of filename="
                  int endPos =
                      esp3d_string::find(contentBuffer.c_str(), "\"", startPos);
                  if (endPos == -1) {
                    esp3d_log_e(
                        "Error parsing content disposition,missing name "
                        "parameter");
                    esp3dHttpService.pushError(ESP3DUploadError::upload,
                                               "Upload error");
                    hasError = true;
                  }
                  fileName = contentBuffer.substr(startPos, endPos - startPos);
                  esp3d_log("Got filename=%s", fileName.c_str());
                }

                parsing_state = ESP3DParseState::content_type;
                contentBuffer = "";
              } else {
                esp3d_log_e(
                    "Error parsing content disposition, wrong end of line");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              }
            } else {
              if (contentBuffer.length() > 300) {
                esp3d_log_e("Error parsing content disposition, wrong size");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              } else {
                contentBuffer += packet[pIndex];
              }
            }
            break;
          case ESP3DParseState::content_type:
            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (packet[pIndex] == '\n') {
              if (prevChar == '\r') {
                esp3d_log("Got %s", contentBuffer.c_str());
                esp3d_log("size: %d", contentBuffer.length());
                if (contentBuffer.length() == 0) {
                  if (fileName.length() > 0) {
                    parsing_state = ESP3DParseState::data_file;
                  } else {
                    parsing_state = ESP3DParseState::data_form;
                    uploadState = ESP3DUploadState::upload_start;
                  }
                } else {
                  parsing_state = ESP3DParseState::content_separator;
                }
                contentBuffer = "";
              } else {
                esp3d_log_e(
                    "Error parsing content disposition, wrong end of line");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              }
            } else {
              if (contentBuffer.length() < 255) {
                if (packet[pIndex] != '\r') {
                  contentBuffer += packet[pIndex];
                }
              } else {
                esp3d_log_e("Error parsing content type, wrong size");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              }
            }

            break;
          case ESP3DParseState::content_separator:
            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (packet[pIndex] == '\n' || packet[pIndex] == '\r') {
              if (packet[pIndex] == '\n' && prevChar == '\r') {
                if (fileName.length() > 0) {
                  parsing_state = ESP3DParseState::data_file;
                  if (!packetWrite || !post_upload_ctx->writeFn) {
                    esp3d_log_e("This url does not allow upload");
                    esp3dHttpService.pushError(ESP3DUploadError::upload,
                                               "Upload error");
                    hasError = true;
                  }
                } else {
                  parsing_state = ESP3DParseState::data_form;
                }
              }
            } else {
              esp3d_log_e(
                  "Error parsing content disposition, wrong end of line");
              esp3dHttpService.pushError(ESP3DUploadError::upload,
                                         "Upload error");
              hasError = true;
            }
            break;
          case ESP3DParseState::data_form:
            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (packet[pIndex] == '\n') {
              if (prevChar == '\r') {
                esp3d_log("Got %s=%s", argName.c_str(), argValue.c_str());
                post_upload_ctx->args.push_back(
                    std::make_pair(argName, argValue));
                argName = "";
                argValue = "";
                parsing_state = ESP3DParseState::boundary;
                boundaryCursor = 2;
              } else {
                esp3d_log_e("Error parsing data form, wrong end of line");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              }
            } else {
              if (argValue.length() < 255) {
                if (packet[pIndex] != '\r' && argValue.length() < 255) {
                  argValue += packet[pIndex];
                }
              } else {
                esp3d_log_e("Error parsing content type, wrong size");
                esp3dHttpService.pushError(ESP3DUploadError::upload,
                                           "Upload error");
                hasError = true;
              }
            }
            break;
          case ESP3DParseState::data_file:

            // esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex],
            // packet[pIndex]);
            if (uploadState == ESP3DUploadState::upload_start) {
              esp3d_log("File part Start");
              // if path and path not visible in filenameValue
              // re-generate filenameValue with path
              if (esp3dHttpService.hasArg(req, "path")) {
                std::string path = esp3dHttpService.getArg(req, "path");
                esp3d_log("Path from post: %s", path.c_str());
                if (!esp3d_string::startsWith(fileName.c_str(), path.c_str())) {
                  if (path[path.length() - 1] != '/') {
                    path += "/";
                  }
                  fileName = path + fileName;
                }
              }
              // Check if have filename size in args
              // if yes update filesize accordingly
              std::string fileSizeArg = fileName + "S";
              if (esp3dHttpService.hasArg(req, fileSizeArg.c_str())) {
                fileSize =
                    atoi(esp3dHttpService.getArg(req, fileSizeArg.c_str()));
                esp3d_log("File size from post: %d", fileSize);
              }
              if (post_upload_ctx->writeFn) {
                if (ESP_OK !=
                    post_upload_ctx->writeFn((const uint8_t *)nullptr, 0,
                                             ESP3DUploadState::upload_start,
                                             fileName.c_str(), fileSize)) {
                  esp3d_log_e("Error writing file invalid");
                  uploadState = ESP3DUploadState::upload_aborted;
                  hasError = true;
                }
              }
              uploadState = ESP3DUploadState::file_write;
              boundaryCursor = 0;
            }

            if (packet[pIndex] == boundaryString[boundaryCursor]) {
              // esp3d_log("Boundary index %d found", boundaryCursor);
              boundaryCursor++;
              // this is boundary string but not final one
              if (boundaryCursor == boundaryString.length()) {
                uploadState = ESP3DUploadState::upload_end;
                boundaryCursor = 0;
                parsing_state = ESP3DParseState::content_disposition;
              }
            } else {
              if (boundaryCursor == 0) {
                // esp3d_log("Look data %d %d", packet[pIndex], packet[pIndex]);
                packetWrite[indexPacketWrite] = packet[pIndex];
                indexPacketWrite++;
              } else {
                // esp3d_log("it is not data %d %d and left %ld",
                // packet[pIndex], packet[pIndex], notProcessed);
                if (packet[pIndex] == '-' && notProcessed < 4) {
                  esp3d_log("End of transfert found");
                  uploadState = ESP3DUploadState::upload_end;
                  parsing_state = ESP3DParseState::boundary;
                } else {
                  // the data looks like begining of boundary but it is not
                  // finalized so copy boundary part to write buffer can be /r/n
                  // of the file content and any additional data identical to
                  // boundary start part esp3d_log("It is data but see as
                  // boundary so drop boundary part");
                  for (uint c = 0; c < boundaryCursor && !hasError; c++) {
                    packetWrite[indexPacketWrite] = boundaryString[c];
                    indexPacketWrite++;
                    if (indexPacketWrite == post_upload_ctx->packetWriteSize) {
                      if (post_upload_ctx->writeFn) {
                        if (ESP_OK != post_upload_ctx->writeFn(
                                          (const uint8_t *)packetWrite,
                                          indexPacketWrite,
                                          ESP3DUploadState::file_write,
                                          fileName.c_str(), fileSize)) {
                          esp3d_log_e("Error writing file invalid");
                          hasError = true;
                        }
                      }
                      indexPacketWrite = 0;
                    }
                  }
                  // reprocess current char with new context
                  boundaryCursor = 0;
                  pIndex--;
                  notProcessed++;
                  // esp3d_log("Revert notprocess to %ld", notProcessed);
                  continue;
                }
              }
            }
            if (indexPacketWrite == post_upload_ctx->packetWriteSize ||
                (uploadState == ESP3DUploadState::upload_end &&
                 indexPacketWrite > 0)) {
              // esp3d_log("Write %d bytes", indexPacketWrite);
              if (post_upload_ctx->writeFn) {
                if (ESP_OK !=
                    post_upload_ctx->writeFn((const uint8_t *)packetWrite,
                                             indexPacketWrite,
                                             ESP3DUploadState::file_write,
                                             fileName.c_str(), fileSize)) {
                  esp3d_log_e("Error writing file invalid");
                  hasError = true;
                }
              }
              indexPacketWrite = 0;
            }

            if (uploadState == ESP3DUploadState::upload_end) {
              // send end of file
              if (post_upload_ctx->writeFn) {
                if (ESP_OK !=
                    post_upload_ctx->writeFn((const uint8_t *)nullptr, 0,
                                             ESP3DUploadState::upload_end,
                                             fileName.c_str(), fileSize)) {
                  esp3d_log_e("Error final file is invalid");
                  hasError = true;
                }
              }
              uploadState = ESP3DUploadState::upload_start;
              fileName = "";
              fileSize = -1;
            }

            break;
          default:
            break;
        }
        prevChar = packet[pIndex];
      }
    }
  } else {
    // not supported so close connection
    esp3d_log_e("Error not supported");
    esp3dHttpService.pushError(ESP3DUploadError::upload, "Upload error");
    hasError = true;
  }

  // Free memory
  if (packet) {
    free(packet);
    packet = nullptr;
  }
  if (packetWrite) {
    free(packetWrite);
    packetWrite = nullptr;
  }
  esp3d_log("Upload done : %s, bytes not processed :%ld ",
            hasError ? "Error" : "Success", notProcessed);
  if (notProcessed <= 0 && !hasError) {
    // it is last boundary we assume the 4 last chars are `--\r\n`
    // at this stage if not the case it is not really an issue anymore
    esp3d_log("Now go to new request handle");
#if ESP3D_TFT_BENCHMARK
    float timesec = (1.0 * (esp_timer_get_time() - startBenchmark)) / 1000000;
    esp3d_report("duration %.2f seconds for %d bytes = %.2f KB/s", timesec,
                 (size_t)(req->content_len),
                 ((1.0 * (size_t)(req->content_len)) / timesec) / 1024);
#endif  // ESP3D_TFT_BENCHMARK
    return (post_upload_ctx->nextHandler(req));
  }
  // send abort state to writefn
  if (post_upload_ctx->writeFn) {
    post_upload_ctx->writeFn((const uint8_t *)nullptr, 0,
                             ESP3DUploadState::upload_aborted, fileName.c_str(),
                             fileSize);
  }
  return httpd_resp_send_500(req);
}
