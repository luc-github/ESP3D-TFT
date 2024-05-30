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

#pragma once
#include <esp_http_server.h>
#include <stdio.h>

#include <list>
#include <utility>

#include "authentication/esp3d_authentication_types.h"
#include "esp3d_client.h"
#include "esp3d_string.h"
#if ESP3D_WEBDAV_SERVICES_FEATURE
#include "webdav/esp3d_webdav_service.h"
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
#include "tasks_def.h"

#define CHUNK_BUFFER_SIZE STREAM_CHUNK_SIZE

#ifdef __cplusplus
extern "C" {
#endif

// for file upload
enum class ESP3DUploadState : uint8_t {
  upload_start,
  file_write,
  upload_end,
  upload_aborted,
};

// for complete upload (several files in upload)
enum class ESP3DUploadStatus : uint8_t {
  not_started,
  on_going,
  error,
  success
};

enum class ESP3DUploadError : uint8_t {
  // Errors code
  no_error = 0,
  authentication_failed,
  file_create_failed,
  write_failed,
  upload,
  not_enough_space,
  start_update_failed,
  close_failed,
  no_sd,
  mount_sd_failed,
  reset_numbering,  // not used
  memory_allocation,
  access_denied,
  wrong_size,
  update_failed,
};

enum class esp3dSocketType : uint8_t {
  unknown,
  http,
  websocket_webui,
  websocket_data,
};

#define HTTPD_401 "401 UNAUTHORIZED" /*!< HTTP Response 401 */

struct PostUploadContext {
  esp_err_t (*writeFn)(const uint8_t *data, size_t datasize,
                       ESP3DUploadState file_upload_state, const char *filename,
                       size_t filesize);
  esp_err_t (*nextHandler)(httpd_req_t *req);
  uint packetReadSize;
  uint packetWriteSize;
  ESP3DUploadStatus status;  // FixMe: is that necessary? currently not used
  std::list<std::pair<std::string, std::string>> args;
};

class ESP3DHttpService final {
 public:
  ESP3DHttpService();
  ~ESP3DHttpService();
  bool begin();
  void handle();
  void end();
  void process(ESP3DMessage *msg);
  bool started() { return _started; };
  httpd_handle_t getServerHandle() { return _server; };
  void pushError(ESP3DUploadError errcode, const char *st);
#if ESP3D_AUTHENTICATION_FEATURE
  static esp_err_t not_authenticated_handler(httpd_req_t *req);
  static char *generate_http_auth_basic_digest(const char *username,
                                               const char *password);
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  static esp_err_t httpd_resp_set_http_hdr(httpd_req_t *req);
  static ESP3DAuthenticationLevel getAuthenticationLevel(httpd_req_t *req);
  static esp_err_t root_get_handler(httpd_req_t *req);
  static esp_err_t command_handler(httpd_req_t *req);
  static esp_err_t config_handler(httpd_req_t *req);
#if ESP3D_SSDP_FEATURE
  static esp_err_t description_xml_handler(httpd_req_t *req);
#endif  // #if ESP3D_SSDP_FEATURE
  static esp_err_t favicon_ico_handler(httpd_req_t *req);
  static esp_err_t file_not_found_handler(httpd_req_t *req,
                                          httpd_err_code_t err);
  static esp_err_t login_handler(httpd_req_t *req);
  static esp_err_t files_handler(httpd_req_t *req);
#if ESP3D_CAMERA_FEATURE
  static esp_err_t snap_handler(httpd_req_t *req);
#endif  // ESP3D_CAMERA_FEATURE
#if ESP3D_SD_CARD_FEATURE
  static esp_err_t sdfiles_handler(httpd_req_t *req);
  static esp_err_t upload_to_sd_handler(const uint8_t *data, size_t datasize,
                                        ESP3DUploadState file_upload_state,
                                        const char *filename, size_t filesize);
#endif  // ESP3D_SD_CARD_FEATURE
  static esp_err_t websocket_webui_handler(httpd_req_t *req);
#if ESP3D_WS_SERVICE_FEATURE
  static esp_err_t websocket_data_handler(httpd_req_t *req);
#endif  // ESP3D_WS_SERVICE_FEATURE
#if ESP3D_WEBDAV_SERVICES_FEATURE
  static esp_err_t webdav_get_handler(httpd_req_t *req);
  static esp_err_t webdav_copy_handler(httpd_req_t *req);
  static esp_err_t webdav_move_handler(httpd_req_t *req);
  static esp_err_t webdav_delete_handler(httpd_req_t *req);
  static esp_err_t webdav_mkcol_handler(httpd_req_t *req);
  static esp_err_t webdav_propfind_handler(httpd_req_t *req);
  static esp_err_t webdav_put_handler(httpd_req_t *req);
  static esp_err_t webdav_head_handler(httpd_req_t *req);
  static esp_err_t webdav_options_handler(httpd_req_t *req);
  static esp_err_t webdav_lock_handler(httpd_req_t *req);
  static esp_err_t webdav_unlock_handler(httpd_req_t *req);
  static esp_err_t webdav_proppatch_handler(httpd_req_t *req);

  static esp_err_t http_send_response(httpd_req_t *req, int code,
                                      const char *msg);
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE

  static esp_err_t post_multipart_handler(httpd_req_t *req);
  static esp_err_t upload_to_flash_handler(const uint8_t *data, size_t datasize,
                                           ESP3DUploadState file_upload_state,
                                           const char *filename,
                                           size_t filesize);
#if ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
  static uint showAllHeaders(httpd_req_t *req);
#endif  // ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
#if ESP3D_UPDATE_FEATURE
  static esp_err_t updatefw_handler(httpd_req_t *req);
  static esp_err_t upload_to_updatefw_handler(
      const uint8_t *data, size_t datasize, ESP3DUploadState file_upload_state,
      const char *filename, size_t filesize);
#endif  // ESP3D_UPDATE_FEATURE

  // static esp_err_t open_fn(httpd_handle_t hd, int socketFd);
  static void close_fn(httpd_handle_t hd, int socketFd);
  void onClose(int socketFd);
  esp_err_t streamFile(const char *path, httpd_req_t *req);
  esp_err_t sendStringChunk(httpd_req_t *req, const char *str,
                            bool autoClose = true);
  esp_err_t sendBinaryChunk(httpd_req_t *req, const uint8_t *data, size_t len,
                            bool autoClose = true);
  bool hasArg(httpd_req_t *req, const char *argname);
  void push(esp3dSocketType socketType, int socketFd);
  void pop(esp3dSocketType socketType, int socketFd);

  const char *getArg(httpd_req_t *req, const char *argname);
  uint32_t getPort() { return _port; };
  bool webdavActive(bool fromSettings = false);

 private:
#if ESP3D_WEBDAV_SERVICES_FEATURE
  bool _webdav_active;
#endif  // ESP3D_WEBDAV_SERVICES_FEATURE
  static char _chunk[CHUNK_BUFFER_SIZE];
  bool _started;
  httpd_handle_t _server;
  uint32_t _port;
  const char *getBoundaryString(httpd_req_t *req);
  // it is based on : `only one post is supported at once`
  static PostUploadContext _post_files_upload_ctx;
#if ESP3D_SD_CARD_FEATURE
  static PostUploadContext _post_sdfiles_upload_ctx;
#endif  // ESP3D_SD_CARD_FEATURE
  static int _clearPayload(httpd_req_t *req);
#if ESP3D_UPDATE_FEATURE
  static PostUploadContext _post_updatefw_upload_ctx;
#endif  // ESP3D_UPDATE_FEATURE

  static PostUploadContext _post_login_ctx;
  std::list<std::pair<esp3dSocketType, int>> _sockets_list;
};

extern ESP3DHttpService esp3dHttpService;
#ifdef __cplusplus
}  // extern "C"
#endif
