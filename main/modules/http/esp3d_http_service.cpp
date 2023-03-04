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


#include "esp3d_http_service.h"
#include "tasks_def.h"
#include <stdio.h>
#include "esp_tls_crypto.h"
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "network/esp3d_network.h"
#include "filesystem/esp3d_globalfs.h"
#include "websocket/esp3d_ws_service.h"
#include "websocket/esp3d_webui_service.h"

#define CHUNK_BUFFER_SIZE STREAM_CHUNK_SIZE
char chunk[CHUNK_BUFFER_SIZE];

Esp3DHttpService esp3dHttpService;

post_upload_ctx_t Esp3DHttpService::_post_files_upload_ctx= {
    .writeFn= (esp_err_t (*)(const uint8_t *, size_t,esp3d_upload_state_t, const char *, size_t ))(Esp3DHttpService::upload_to_flash_handler),
    .nextHandler= (esp_err_t (*)(httpd_req_t*))(Esp3DHttpService::files_handler),
    .packetReadSize = 4* 1024, //This may need to be defined in tasks_def.h
    .packetWriteSize= 4* 1024,  //This may need to be defined in tasks_def.h
    .status = upload_not_started,
    .args = {}
};

post_upload_ctx_t Esp3DHttpService::_post_sdfiles_upload_ctx= {
    .writeFn= (esp_err_t (*)(const uint8_t *, size_t,esp3d_upload_state_t, const char *, size_t ))(Esp3DHttpService::upload_to_sd_handler),
    .nextHandler= (esp_err_t (*)(httpd_req_t*))(Esp3DHttpService::sdfiles_handler),
    .packetReadSize = 4* 1024,  //This may need to be defined in tasks_def.h
    .packetWriteSize= 4* 1024, //This may need to be defined in tasks_def.h
    .status = upload_not_started,
    .args = {}
};

post_upload_ctx_t Esp3DHttpService::_post_updatefw_upload_ctx= {
    .writeFn= (esp_err_t (*)(const uint8_t *, size_t,esp3d_upload_state_t, const char *, size_t ))(Esp3DHttpService::upload_to_updatefw_handler),
    .nextHandler= (esp_err_t (*)(httpd_req_t*))(Esp3DHttpService::updatefw_handler),
    .packetReadSize = 1024,  //This may need to be defined in tasks_def.h
    .packetWriteSize= 1024,  //This may need to be defined in tasks_def.h
    .status = upload_not_started,
    .args = {}
};

post_upload_ctx_t Esp3DHttpService::_post_login_ctx= {
    .writeFn= NULL,
    .nextHandler= (esp_err_t (*)(httpd_req_t*))(Esp3DHttpService::login_handler),
    .packetReadSize = 512,  //This may need to be defined in tasks_def.h
    .packetWriteSize= 0,  //This may need to be defined in tasks_def.h
    .status = upload_not_started,
    .args = {}
};


void Esp3DHttpService::pushError(esp3d_http_error_t errcode, const char * st)
{
    std::string errmsg = "ERROR:" + std::to_string(errcode);
    errmsg+= ":";
    errmsg+=st;
    errmsg+= "\n";
    esp3dWsWebUiService.BroadcastTxt((uint8_t*)errmsg.c_str(), strlen(errmsg.c_str()));
}

void Esp3DHttpService::push (esp3d_http_socket_t socketType, int socketFd)
{
    _sockets_list.push_back(std::make_pair(socketType,socketFd));
}

void Esp3DHttpService::pop (esp3d_http_socket_t socketType, int socketFd)
{
    _sockets_list.remove_if([&](std::pair<esp3d_http_socket_t, int>& p) {
        return p.first== socketType and p.second==socketFd;
    });
}

bool Esp3DHttpService::hasArg(httpd_req_t *req, const char* argname)
{
    post_upload_ctx_t *post_upload_ctx = (post_upload_ctx_t *)req->user_ctx;
    if (post_upload_ctx) {
        for(auto itr=post_upload_ctx->args.begin(); itr!=post_upload_ctx->args.end(); itr++) {
            if (strcmp(itr->first.c_str(), argname)==0) {
                return true;
            }
        }
    }

    return false;
}

const char * Esp3DHttpService::getArg(httpd_req_t *req, const char* argname)
{
    post_upload_ctx_t *post_upload_ctx = (post_upload_ctx_t *)req->user_ctx;
    if (post_upload_ctx) {
        for(auto itr=post_upload_ctx->args.begin(); itr!=post_upload_ctx->args.end(); itr++) {
            if (strcmp(itr->first.c_str(), argname)==0) {
                return itr->second.c_str();
            }
        }
    }
    return "";
}

Esp3DHttpService::Esp3DHttpService()
{
    _started = false;
    _server = nullptr;
    _post_files_upload_ctx.status = upload_not_started;
}

Esp3DHttpService::~Esp3DHttpService()
{
    end();
}

/*esp_err_t Esp3DHttpService::open_fn(httpd_handle_t hd, int socketFd)
{
    esp3d_log("New client connection %d", socketFd);
    return ESP_OK;
}*/

void Esp3DHttpService::close_fn(httpd_handle_t hd, int socketFd)
{
    esp3d_log("Client closing connection %d", socketFd);
    //each service should know if the socket is owned by  itself
    esp3dWsWebUiService.onClose(socketFd);
    esp3dWsDataService.onClose(socketFd);
    close(socketFd );
}

void Esp3DHttpService::onClose(int socketFd)
{
#if ESP3D_AUTHENTICATION_FEATURE
    esp3d_log_w("Closing client connection %d and all WEBUI_CLIENT", socketFd);
    esp3dAuthenthicationService.clearSessions(WEBUI_CLIENT);
#endif //#if ESP3D_AUTHENTICATION_FEATURE
}

bool Esp3DHttpService::begin()
{
    esp3d_log("Starting Http Service");

    end();
    //check if start
    if (esp3d_state_on!= (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_http_on)) {
        esp3d_log("Http is not enabled");
        //return true because no error but _started is false
        return true;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    uint32_t  intValue = esp3dTFTsettings.readUint32(esp3d_http_port);
    //HTTP port
    config.server_port = intValue;
    //Http server core
    config.core_id = 0;
    //stack size (default 4096)
    config.stack_size = 1024*8;
    //Nb of sockets//need to match max LWIP socket in sdkconfig +3 internal
    config.max_open_sockets = 8; //(3 (http) +2 (1+reject)webui +3 (2 + reject) webdata)
    //handler
    config.max_uri_handlers = 12; //currently use 10
    //backlog_conn
    config.backlog_conn       = 8;
    config.close_fn = close_fn;
    config.lru_purge_enable = true;

    //start server
    esp3d_log("Starting server on port: '%d'", config.server_port);
    esp_err_t err = httpd_start(&_server, &config);
    if (err == ESP_OK) {
        // Set URI handlers
        esp3d_log("Registering URI handlers");
        //favicon.ico
        const httpd_uri_t favicon_handler_config = {
            .uri       = "/favicon.ico",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.favicon_ico_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &favicon_handler_config);
#if ESP3D_SSDP_FEATURE
        //description.xml (ssdp)
        const httpd_uri_t ssdp_handler_config = {
            .uri       = "/description.xml",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.description_xml_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &ssdp_handler_config);
#endif //ESP3D_SSDP_FEATURE

        //root /
        const httpd_uri_t root_handler_config = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.root_get_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &root_handler_config);
        //Command /command
        const httpd_uri_t command_handler_config = {
            .uri       = "/command",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.command_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &command_handler_config);

        //config /config
        const httpd_uri_t config_handler_config = {
            .uri       = "/config",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.config_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &config_handler_config);

        //flash files /files
        const httpd_uri_t files_handler_config = {
            .uri       = "/files",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.files_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &files_handler_config);

        //login
        const httpd_uri_t login_handler_config = {
            .uri       = "/login",
            .method    = HTTP_POST,
            .handler   = post_multipart_handler,
            .user_ctx  =  &_post_login_ctx,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &login_handler_config);

        //flash files upload (POST data)
        httpd_uri_t files_upload_handler_config = {
            .uri       = "/files",   // Match all URIs of type /upload/path/to/file
            .method    = HTTP_POST,
            .handler   = post_multipart_handler,
            .user_ctx  =  &_post_files_upload_ctx,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &files_upload_handler_config);

        //sdfiles upload (POST data)
        httpd_uri_t sdfiles_upload_handler_config = {
            .uri       = "/sdfiles",   // Match all URIs of type /upload/path/to/file
            .method    = HTTP_POST,
            .handler   = post_multipart_handler,
            .user_ctx  =  &_post_sdfiles_upload_ctx,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &sdfiles_upload_handler_config);

        //updatefw upload (POST data)
        httpd_uri_t updatefw_upload_handler_config = {
            .uri       = "/updatefw",   // Match all URIs of type /upload/path/to/file
            .method    = HTTP_POST,
            .handler   = post_multipart_handler,
            .user_ctx  =  &_post_updatefw_upload_ctx,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &updatefw_upload_handler_config);

        //sd files /sdfiles
        const httpd_uri_t sdfiles_handler_config = {
            .uri       = "/sdfiles",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.sdfiles_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &sdfiles_handler_config);

        //webui web socket /ws
        const httpd_uri_t websocket_webui_handler_config = {
            .uri       = "/ws",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.websocket_webui_handler),
            .user_ctx  =  nullptr,
            .is_websocket = true,
            .handle_ws_control_frames = false,
            .supported_subprotocol = "webui-v3"
        };
        httpd_register_uri_handler(_server, &websocket_webui_handler_config);

        const httpd_uri_t websocket_data_handler_config = {
            .uri       = "/wsdata",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.websocket_data_handler),
            .user_ctx  =  nullptr,
            .is_websocket = true,
            .handle_ws_control_frames = false,
            .supported_subprotocol = "arduino"
        };
        httpd_register_uri_handler(_server, &websocket_data_handler_config);

        //File not found
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, (httpd_err_handler_func_t )file_not_found_handler);

        //websocket service
        esp3d_websocket_config_t wsConfig= {
            .serverHandle=_server,
            .max_clients = 1,
            .type=ESP3D_WS_WEBUI_SOCKET
        };
        _started = esp3dWsWebUiService.begin(&wsConfig);
        if (_started) {
            if (esp3d_state_on!= (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_ws_on)) {
                esp3d_log("WS is not enabled");
            } else {
                wsConfig.max_clients = 2;
                wsConfig.type= ESP3D_WS_DATA_SOCKET;
                _started = esp3dWsDataService.begin(&wsConfig);
            }
        }
    } else {
        esp3d_log_e("Web server start failed %s",esp_err_to_name(err)) ;
    }
    return _started;
}

void Esp3DHttpService::handle() {}

void Esp3DHttpService::end()
{
    if (!_started && !_server) {
        return;
    }
    esp3d_log("Stop Http Service");
    if (_server) {
        esp3dWsWebUiService.end();
        esp3dWsDataService.end();
        httpd_unregister_uri(_server, "/favicon.ico");
        httpd_unregister_uri(_server, "/command");
        httpd_unregister_uri(_server, "/");
        httpd_unregister_uri(_server, "/files");
        httpd_unregister_uri(_server, "/ws");
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, NULL);
        httpd_stop(_server);
    }
    _server = nullptr;
    _started = false;
    _post_files_upload_ctx.status = upload_not_started;
    _post_sdfiles_upload_ctx.status = upload_not_started;
}
#if ESP3D_AUTHENTICATION_FEATURE
char *Esp3DHttpService::generate_http_auth_basic_digest(const char *username, const char *password)
{
    int out;
    char *user_info = NULL;
    char *digest = NULL;
    size_t n = 0;
    asprintf(&user_info, "%s:%s", username, password);
    if (!user_info) {
        esp3d_log_e("No enough memory for user information");
        return NULL;
    }
    esp_crypto_base64_encode(NULL, 0, &n, (const unsigned char *)user_info, strlen(user_info));

    /* 6: The length of the "Basic " string
     * n: Number of bytes for a base64 encode format
     * 1: Number of bytes for a reserved which be used to fill zero
    */
    digest = (char *)calloc(1, 6 + n + 1);
    if (digest) {
        strcpy(digest, "Basic ");
        esp_crypto_base64_encode((unsigned char *)digest + 6, n, (size_t *)&out, (const unsigned char *)user_info, strlen(user_info));
    }
    free(user_info);
    return digest;
}
#endif //#if ESP3D_AUTHENTICATION_FEATURE

esp3d_authentication_level_t Esp3DHttpService::getAuthenticationLevel(httpd_req_t *req)
{
#if ESP3D_AUTHENTICATION_FEATURE
    esp3d_authentication_level_t authentication_level = ESP3D_LEVEL_GUEST;
    esp3d_log_w("Checking URI: %s, Authentication level is %d",req->uri,authentication_level);
#endif // ESP3D_AUTHENTICATION_FEATURE

    if ( req->sess_ctx) {
        esp3d_log_w("Context cookie: %s",(char *)req->sess_ctx);
    }
#if ESP3D_AUTHENTICATION_FEATURE
    //get socket
    int socketId =  httpd_req_to_sockfd(req);
    std::string sessionID="";
    struct sockaddr_in6 saddr;   // esp_http_server uses IPv6 addressing
    socklen_t saddr_len = sizeof(saddr);
    //get ip
    if(getpeername(socketId, (struct sockaddr *)&saddr, &saddr_len)>= 0) {
        static char address_str[40];
        inet_ntop(AF_INET, &saddr.sin6_addr.un.u32_addr[3], address_str, sizeof(address_str));
        esp3d_log("client IP is %s", address_str);
        // (( struct sockaddr_in *) (&_clients[freeIndex].source_addr))->sin_addr.s_addr = saddr.sin6_addr.un.u32_addr[3];
    } else {
        esp3d_log_e("Failed to get address for new connection");
    }
    //  1 - check post parameter SUBMIT + USER + PASSWORD
    post_upload_ctx_t *post_upload_ctx = (post_upload_ctx_t *)req->user_ctx;
    char *buf = NULL;
    size_t buf_len = 0;
    if (post_upload_ctx) {
        esp3d_log("Check post parameters: SUBMIT + USER + PASSWORD");
        if (esp3dHttpService.hasArg(req,"SUBMIT") && esp3dHttpService.hasArg(req,"USER") && esp3dHttpService.hasArg(req,"PASSWORD")) {
            std::string tmpstr = esp3dHttpService.getArg(req,"SUBMIT");
            if (tmpstr == "YES") {
                esp3dHttpService.onClose(socketId);
                tmpstr = esp3dHttpService.getArg(req,"USER");
                if (tmpstr == "user") {
                    if (esp3dAuthenthicationService.isUser(esp3dHttpService.getArg(req,"PASSWORD"))) {
                        esp3d_log("Post user authentication is user");
                        authentication_level = ESP3D_LEVEL_USER;
                    }
                } else if (tmpstr == "admin") {
                    if (esp3dAuthenthicationService.isAdmin(esp3dHttpService.getArg(req,"PASSWORD"))) {
                        esp3d_log("Post user authentication is admin");
                        authentication_level = ESP3D_LEVEL_ADMIN;
                    }
                }
            }
        }
    }
    //if authentication_level is still ESP3D_LEVEL_GUEST
    //2 - check Autorization
    if (authentication_level == ESP3D_LEVEL_GUEST) {
        buf = NULL;
        buf_len = 0;
        buf_len = httpd_req_get_hdr_value_len(req, "Authorization") + 1;
        if (buf_len > 1) {
            esp3d_log("Check Basic authentication based on Autorization");
            buf = (char *)calloc(1, buf_len);
            if (buf) {
                if (httpd_req_get_hdr_value_str(req, "Authorization", buf, buf_len) == ESP_OK) {
                    esp3d_log("Found header => Authorization: %s", buf);
                    esp3dHttpService.onClose(socketId);
                    char *auth_credentials = generate_http_auth_basic_digest("admin",esp3dAuthenthicationService.getAdminPassword());
                    if (auth_credentials) {
                        if (strncmp(auth_credentials, buf, buf_len)==0) {
                            esp3d_log("Authorizaton user authentication is admin");
                            authentication_level = ESP3D_LEVEL_ADMIN;
                        } else {
                            free(auth_credentials);
                            auth_credentials = generate_http_auth_basic_digest("user",esp3dAuthenthicationService.getUserPassword());
                            if (auth_credentials) {
                                if (strncmp(auth_credentials, buf, buf_len)==0) {
                                    esp3d_log("Authorizaton user authentication is user");
                                    authentication_level = ESP3D_LEVEL_USER;
                                }
                            } else {
                                esp3d_log_e("Failed to generate user auth credentials");
                            }
                        }
                        if (auth_credentials) {
                            free(auth_credentials);
                        }
                    } else {
                        esp3d_log_e("Failed to generate admin auth credentials");
                    }
                } else {
                    esp3d_log_e("No enough memory for basic authorization");
                }
                if (buf) {
                    free(buf);
                    buf = NULL;
                }
                buf_len = 0;
            } else {
                esp3d_log("No auth value received");
            }
        } else {
            esp3d_log("No auth value received");
        }
    }
    //Check if have sessionID in Cookie header
    char cookie_str[128];
    size_t cookie_len = 127;
    esp_err_t res= httpd_req_get_cookie_val(req, "ESPSESSIONID", cookie_str, &cookie_len);
    esp3d_log("Has header Cookie size %d", cookie_len);
    if (res == ESP_OK) {
        sessionID = cookie_str;
        esp3d_log("Cookie Session ID: %s size: %d", sessionID.c_str(),sessionID.length());
        if (sessionID.length()!=24) {
            esp3d_log("Session ID is not valid");
            sessionID.clear();
        }
    } else {
        esp3d_log_w("Failed to get sessionID, err %s", esp_err_to_name(res));
        sessionID.clear();
    }
    esp3d_log("Authentication level is %d", authentication_level);
//if authentication_level is not ESP3D_LEVEL_GUEST
//Create / update Session ID and cookie
    if (authentication_level != ESP3D_LEVEL_GUEST) {
        esp3d_log("Authentication level is %d", authentication_level);
        //Authentication level is not ESP3D_LEVEL_GUEST
        //check if session ID is already set
        if (sessionID.length()!=0) {
            //Update Session ID
            esp3d_log("Found Session ID: %s",sessionID.c_str());
            esp3d_authentication_record_t * rec  =  esp3dAuthenthicationService.getRecord(sessionID.c_str());
            if (rec) {
                //we have a record for this session ID so update it
                //update timeout
                //update level
                esp3d_log("Record created for level %d",authentication_level);
                rec->level = authentication_level;
                rec->last_time = esp3d_hal::millis();
            } else {
                //We have no record for this session ID so create one
                if ( esp3dAuthenthicationService.createRecord (sessionID.c_str(), socketId, authentication_level, WEBUI_CLIENT)) {
                    esp3d_log("Record created for level %d",authentication_level);
                } else {
                    esp3d_log_e("Failed to create session id");
                    //TBD:
                    //Should  we reset authencation level to guest?
                }
            }
        } else {
            //create new session ID
            struct sockaddr_storage source_addr;
            ((struct sockaddr_in *)&source_addr)->sin_addr.s_addr=saddr.sin6_addr.un.u32_addr[3];
            sessionID = esp3dAuthenthicationService.create_session_id(source_addr, socketId);
            esp3d_log("Create Session ID: %s",sessionID.c_str());
            //update record list
            if ( esp3dAuthenthicationService.createRecord (sessionID.c_str(), socketId, authentication_level, WEBUI_CLIENT)) {
                esp3d_log("Record created for level %d",authentication_level);
                //Add Cookie to session ID

                std::string cookie = "ESPSESSIONID=" + sessionID;
                if (! req->sess_ctx) {
                    esp3d_log("Add context cookie to session");
                    req->sess_ctx = malloc(sizeof(char)*cookie.length()+1);
                } else {
                    esp3d_log_e("Memory error adding cookie to session");
                }
                if (req->sess_ctx) {
                    strcpy((char *)req->sess_ctx, cookie.c_str());
                }
                esp3d_log_e("add set cookie header: %s", (char*)req->sess_ctx );
                if (ESP_OK!=httpd_resp_set_hdr(req, "Set-Cookie",(char*)req->sess_ctx )) {
                    esp3d_log_e("Failed to set cookie header");
                }
            } else {
                esp3d_log_e("Failed to create session id");
                //TBD:
                //Should  we reset authencation level to guest?
            }
        }
    } else {
        //Authentication level still is ESP3D_LEVEL_GUEST
        esp3d_log("Authentication level still is ESP3D_LEVEL_GUEST");
        if (sessionID.length()!=0) {
            // we have a session ID
            //check if time out is set
            esp3d_log("Got Session ID: %s",sessionID.c_str());
            esp3d_authentication_record_t * rec  =  esp3dAuthenthicationService.getRecord(sessionID.c_str());
            if (rec) {
                if (esp3dAuthenthicationService.getSessionTimeout() !=0) { //no session limit
                    if (esp3d_hal::millis() - rec->last_time < esp3dAuthenthicationService.getSessionTimeout()) {
                        esp3d_log("Update Session timeout");
                        rec->last_time=esp3d_hal::millis();
                        esp3d_log("Authentication level now %d", rec->level);
                        authentication_level =rec->level;

                    } else { // session  reached limit
                        esp3d_log_w("Session is now outdated %lld vs %lld",esp3d_hal::millis() - rec->last_time, esp3dAuthenthicationService.getSessionTimeout());
                        authentication_level = ESP3D_LEVEL_GUEST;
                        //Update cookie to 0
                        esp3d_log_e("add set cookie header: %s", "ESPSESSIONID=0");
                        if (ESP_OK!=httpd_resp_set_hdr(req, "Set-Cookie", "ESPSESSIONID=0")) {
                            esp3d_log_e("Failed to set cookie header");
                        }

                        //delete session
                        if (!esp3dAuthenthicationService.clearSession(sessionID.c_str())) {
                            esp3d_log_e("Failed to clear session");
                        }
                    }
                } else {
                    //in case setting change
                    rec->last_time=esp3d_hal::millis();
                    esp3d_log("Session has no timeout");
                }
            } else {
                //No record found
                esp3d_log_e("No record found for session id %s",sessionID.c_str());
                //Update cookie to 0
                esp3d_log_e("add set cookie header: %s", "ESPSESSIONID=0");
                if (ESP_OK!=httpd_resp_set_hdr(req, "Set-Cookie", "ESPSESSIONID=0")) {
                    esp3d_log_e("Failed to set cookie header");
                }

            }
        }
    }
    esp3d_log_w("URI: %s, Authentication level is %d",req->uri,authentication_level);
    return authentication_level;
#else
    return ESP3D_LEVEL_ADMIN;
#endif //#if ESP3D_AUTHENTICATION_FEATURE
}
esp_err_t Esp3DHttpService::streamFile (const char * path,httpd_req_t *req )
{
    esp_err_t res = ESP_OK;
    if (!_started|| !_server) {
        esp3d_log_e("Stream server is not ready");
        return ESP_ERR_INVALID_STATE;
    }
    std::string filename = "";
    std::string filenameGz = "";
    bool isGzip = false;
    //check if filename is provided or need to extract from request string
    if (path) {  // this one is already correct no need to decode it neither append mount point
        filename = path;
        filenameGz = filename+".gz";
    } else { //extract file name from query and decode it
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        char * buf = (char *)malloc(buf_len);
        if (buf) {
            res = httpd_req_get_url_query_str(req, buf, buf_len);
            if (res != ESP_OK) {
                esp3d_log_e("Cannot extract query string from uri: %s", esp_err_to_name(res));
            } else {
                //clear possible parameters
                for (uint i = 0; i < buf_len; i++) {
                    if (buf[i] == '?') {
                        buf[i]=0x0;
                        break;
                    }
                }
                esp3d_fs_types fstype = globalFs.getFSType(buf);
                //assume default file serving is flash
                if (fstype==FS_UNKNOWN) {
                    filename=globalFs.mount_point(FS_FLASH);
                }
                filename += esp3d_strings::urlDecode((const char *)buf);
                filenameGz = filename+".gz";
            }
            free(buf );
        } else {
            esp3d_log_e("Memory allocation failed");
            res= ESP_ERR_NO_MEM;
        }
    }
    if (res== ESP_OK) {
        esp3d_log("File name is %s", filename.c_str());
        if (globalFs.accessFS(filename.c_str())) {
            if (globalFs.exists(filenameGz.c_str())) {
                esp3d_log("File exists and it is gzipped");
                isGzip = true;
                res = ESP_OK;
            } else if (globalFs.exists(filename.c_str())) {
                esp3d_log("File exists");
                res = ESP_OK;
            } else {
                esp3d_log_e("File %s does not exists", filename.c_str());
                res = ESP_ERR_NOT_FOUND;
            }
            if (res == ESP_OK) {
                FILE *fd = globalFs.open(isGzip?filenameGz.c_str():filename.c_str(), "r");
                if (fd) {
                    //stream file
                    std::string mimeType= esp3d_strings::getContentType( filename.c_str());

                    httpd_resp_set_type(req, mimeType.c_str());
                    if(isGzip) {
                        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
                    }
                    size_t chunksize;
                    do {
                        chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, fd);
                        if (chunksize > 0) {
                            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                                esp3d_log_e("File sending failed!");
                                chunksize = 0;
                                res = ESP_FAIL;
                            }
                        }
                    } while (chunksize != 0);
                    fclose(fd);
                    httpd_resp_send_chunk(req, NULL, 0);
                } else {
                    res = ESP_ERR_NOT_FOUND;
                    esp3d_log_e("Cannot access File %s", isGzip?filenameGz.c_str():filename.c_str());
                }

            }
            globalFs.releaseFS(filename.c_str());
        } else {
            res = ESP_ERR_NOT_FOUND;
            esp3d_log_e("Cannot access FS");
        }
    }

    return res;
}

void Esp3DHttpService::process(esp3d_msg_t * msg)
{
    if (msg->requestId.httpReq) {
        //esp3d_log("Msg type : %d", msg->type);
        if (httpd_resp_send_chunk(msg->requestId.httpReq, (const char *)msg->data, msg->size) != ESP_OK) {
            httpd_resp_send_chunk(msg->requestId.httpReq, NULL, 0);
            esp3d_log_e("Error sending data, closing chunk");
        } else {
            if (msg->type==  msg_tail || msg->type== msg_unique) {
                httpd_resp_send_chunk(msg->requestId.httpReq, NULL, 0);
                esp3d_log("End of messages for this req, closing chunk");
            }
        }
    }
    Esp3DClient::deleteMsg(msg);
}



esp_err_t Esp3DHttpService::sendStringChunk (httpd_req_t *req, const char * str, bool autoClose )
{
    if (!str || httpd_resp_send_chunk(req, str, strlen(str)) != ESP_OK) {
        esp3d_log_e("String sending failed!");
        if (autoClose) {
            httpd_resp_send_chunk(req, NULL, 0);
        }
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t Esp3DHttpService::sendBinaryChunk (httpd_req_t *req, const uint8_t * data, size_t len, bool autoClose )
{
    if (!data || httpd_resp_send_chunk(req, (const char *)data, len) != ESP_OK) {
        esp3d_log_e("String sending failed!");
        if (autoClose) {
            httpd_resp_send_chunk(req, NULL, 0);
        }
        return ESP_FAIL;
    }
    return ESP_OK;
}

const char * Esp3DHttpService::getBoundaryString (httpd_req_t *req)
{
    static char * boundaryStr = nullptr;
    if (boundaryStr) {
        free(boundaryStr);
    }
    size_t contentTypeHeaderSize =  httpd_req_get_hdr_value_len(req, "Content-Type");
    if (contentTypeHeaderSize) {
        boundaryStr = (char*)calloc(contentTypeHeaderSize+1, sizeof(char));
        if (boundaryStr) {
            esp_err_t r = httpd_req_get_hdr_value_str(req, "Content-Type", boundaryStr, contentTypeHeaderSize+1);
            if (ESP_OK==r) {
                //esp3d_log("Content-Type %s", boundaryStr);
                if (esp3d_strings::startsWith(boundaryStr,"multipart/form-data")) {
                    for(uint i=strlen("multipart/form-data"); i < contentTypeHeaderSize; i++) {
                        if(esp3d_strings::startsWith(&boundaryStr[i],"boundary=")) {
                            return &boundaryStr[i+strlen("boundary=")];
                        }
                    }
                    free(boundaryStr);
                    boundaryStr = nullptr;
                    esp3d_log_e("Boundary not found");
                } else {
                    esp3d_log_e("Not multipart/form-data");
                    free(boundaryStr);
                    boundaryStr =nullptr;
                }
            } else {
                esp3d_log_e("Invalid Content-Type %s", esp_err_to_name(r) );
                free(boundaryStr);
                boundaryStr =nullptr;
            }
        } else {
            esp3d_log_e("Memory allocation failed");
        }
    }
    return boundaryStr;
}
