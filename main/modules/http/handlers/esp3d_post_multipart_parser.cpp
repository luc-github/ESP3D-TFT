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

#include "http/esp3d_http_service.h"
#include <sys/param.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "network/esp3d_network.h"


//TODO fine tune these values and put them in tasks_def.h
#define PACKET_SIZE 1024*1
#define PACKET_WRITE_SIZE 1024*2

char packet[PACKET_SIZE];
char packetWrite[PACKET_WRITE_SIZE];

typedef enum {
    parse_boundary,
    parse_boundary_type,
    parse_content_disposition,
    parse_data_form,
    parse_data_file,
    parse_file_content,
} esp3d_parse_state_t;

esp_err_t Esp3DHttpService::post_multipart_handler(httpd_req_t *req)
{
    esp3d_log("Post Data %d on : %s", req->content_len, req->uri);

    post_upload_ctx_t *post_upload_ctx = (post_upload_ctx_t *)req->user_ctx;
    if (!post_upload_ctx) {
        esp3d_log_e("Context not found");
        return ESP_FAIL;
    }
    if (!(post_upload_ctx->nextHandler)) {
        esp3d_log_e("Post handler not found");
        return ESP_FAIL;
    }
    const char *boundaryPtr = esp3dHttpService.getBoundaryString(req);
    if (!boundaryPtr) {
        esp3d_log_e("No boundary found");
        return ESP_FAIL;
    }
    //be sure list is empty
    post_upload_ctx->args.clear();
    uint8_t boundaryCursor = 0;
    uint8_t boundaryEndCursor = 0;
    bool contentTypePassed = false;
    uint indexPacketWrite=0;
    std::string boundaryString = "--";
    std::string contentBuffer;
    std::string nameFlag = " name=";
    std::string filenameFlag = " filename=";
    char prevChar=0x0;
    std::string argName = "";
    std::string argValue = "";
    std::string filenameValue = "";
    boundaryString += boundaryPtr;
    bool dataFormStarted = false;
    esp3d_log("Boundary is: %s", boundaryString.c_str());
    int remaining = req->content_len;
    int received;
    int fileSize=-1;
    FILE * fileDescriptor = nullptr;
    // processing the content
    if (boundaryPtr) {
        esp3d_parse_state_t parsing_state = parse_boundary;
        while (remaining > 0) {
            if ((received = httpd_req_recv(req, packet, PACKET_SIZE)) <= 0) {
                esp3d_log_e("Connection lost");
                if (parsing_state==parse_file_content && post_upload_ctx->writeFn) {
                    if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_aborted,  fileDescriptor, filenameValue.c_str(), fileSize)) {
                        esp3d_log_e("Error writing file invalid");

                    }
                }
                return ESP_FAIL;
            }
            if (!(received == HTTPD_SOCK_ERR_TIMEOUT || received == HTTPD_SOCK_ERR_INVALID || received == HTTPD_SOCK_ERR_FAIL)) {
                // esp3d_log_e("Purge %d bytes", received);
                remaining -= received;
            }
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                esp3d_log_e("Time out");
                continue;
            }
            if (received == HTTPD_SOCK_ERR_INVALID || received == HTTPD_SOCK_ERR_FAIL) {
                esp3d_log_e("Error connection");
                if (parsing_state==parse_file_content && post_upload_ctx->writeFn) {
                    if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_aborted,  fileDescriptor, filenameValue.c_str(), fileSize)) {
                        esp3d_log_e("Error writing file invalid");

                    }
                }
                return ESP_FAIL;
            }
            for (uint pIndex = 0; pIndex < received; pIndex++) {
                // parse buffer
                switch (parsing_state) {
                case parse_boundary:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (boundaryCursor < boundaryString.length()) {
                        if (boundaryString[boundaryCursor] == packet[pIndex]) {
                            boundaryCursor++;
                        } else {
                            esp3d_log_e("Error parsing boundary");
                            return ESP_FAIL;
                        }
                    } else {
                        if (remaining == 4) {
                            // it is last boundary we assume the 4 last chars are `--\r\n`
                            // at this stage if not the case it is not really an issue anymore
                            esp3d_log("Now go to new request handle");
                            return (post_upload_ctx->nextHandler(req));
                        }
                        if (boundaryEndCursor == 0 && packet[pIndex] == '\r') {
                            boundaryEndCursor++;
                        } else if (boundaryEndCursor == 1 && packet[pIndex] == '\n') {
                            boundaryEndCursor++;
                            esp3d_log("Boundary found");
                            parsing_state = parse_content_disposition;
                            boundaryEndCursor = 0;
                            boundaryCursor = 0;
                        } else {
                            esp3d_log_e("Error parsing end of boundary");
                            return ESP_FAIL;
                        }
                    }
                    break;
                case parse_content_disposition:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (packet[pIndex] == '\n' && contentBuffer[contentBuffer.length() - 1] == '\r') {
                        esp3d_log("Got %s", contentBuffer.c_str());
                        uint startPos = 30; // strlen("Content-Disposition: form-data");
                        //look for name
                        int pos = contentBuffer.find(nameFlag, startPos);
                        if (pos == std::string::npos) {
                            esp3d_log_e("Error parsing name");
                            return ESP_FAIL;
                        }
                        pos+=6;// name="
                        int posEnd;
                        bool endFound = false;
                        // extract name
                        for (posEnd = pos; posEnd < contentBuffer.length() || !endFound; posEnd++) {
                            if (contentBuffer[posEnd] == ';' || contentBuffer[posEnd] == '\n' || contentBuffer[posEnd] == '\r') {
                                endFound = true;
                            } else {
                                if (contentBuffer[posEnd] != '\"') {
                                    argName += contentBuffer[posEnd];
                                }
                            }
                        }
                        if (endFound) {
                            esp3d_log("Got arg:%s", argName.c_str());
                        } else {
                            esp3d_log("Failed to parse name in %s", contentBuffer.c_str());
                            return ESP_FAIL;
                        }

                        //look for filename
                        endFound = false;
                        pos = contentBuffer.find(filenameFlag, startPos);
                        if (pos != std::string::npos) {
                            pos+=10;// filename="
                            // extract name
                            for (posEnd = pos; posEnd < contentBuffer.length() || !endFound; posEnd++) {
                                if (contentBuffer[posEnd] == ';' || contentBuffer[posEnd] == '\n' || contentBuffer[posEnd] == '\r') {
                                    endFound = true;
                                } else {
                                    if (contentBuffer[posEnd] != '\"') {
                                        filenameValue += contentBuffer[posEnd];
                                    }
                                }
                            }
                            if (endFound) {
                                esp3d_log("Got filename:%s", filenameValue.c_str());
                            }
                        }
                        contentBuffer="";
                        if (filenameValue.length() > 0) {
                            esp3d_log("It is file %s", filenameValue.c_str());
                            parsing_state = parse_data_file;
                        } else if (argName.length()>0) {
                            parsing_state = parse_data_form;
                            esp3d_log("It is data form argname is  %s", argName.c_str());
                        } else {
                            esp3d_log_e("Content-Disposition: form-data is invalid");
                            return ESP_FAIL;
                        }
                    } else if (contentBuffer.length() < 512 && packet[pIndex] != '\n') {
                        contentBuffer += packet[pIndex];
                        //esp3d_log("Add %d, %s", packet[pIndex], contentBuffer.c_str());
                    } else {
                        esp3d_log_e("Content-Disposition: form-data is invalid");
                        return ESP_FAIL;
                    }
                    break;
                case parse_data_form:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    //there few chance an arg is same a boundary so look for \r\n
                    if (packet[pIndex] != '\n' && dataFormStarted) {
                        if (packet[pIndex]!='\r') {
                            argValue+=packet[pIndex];
                        }
                        // esp3d_log("Got char add to value %s", argValue.c_str());
                    } else if (!dataFormStarted && packet[pIndex]=='\n') {
                        if (prevChar=='\r') {
                            esp3d_log("Content type passed, starting collect value string");
                            dataFormStarted=true;
                        } else {
                            esp3d_log_e("Content-Disposition: form-data is invalid");
                            return ESP_FAIL;
                        }
                    } else if (dataFormStarted && packet[pIndex]=='\n') {
                        if (prevChar!='\r') {
                            esp3d_log_e("Content-Disposition: form-data is invalid");
                            return ESP_FAIL;
                        }
                        esp3d_log("Found arg value %s:%s", argName.c_str(),argValue.c_str());
                        post_upload_ctx->args.push_back(std::make_pair(argName,argValue));
                        dataFormStarted =false;
                        argName="";
                        argValue="";
                        parsing_state = parse_boundary;
                    } else if (packet[pIndex] != '\n' && !dataFormStarted)  {
                        //ignore data
                        //esp3d_log("ignore char: %c", packet[pIndex]);
                    } else {
                        esp3d_log_e("Data from Content-Disposition section is invalid");
                        return ESP_FAIL;
                    }
                    break;
                case parse_data_file:
                    if (packet[pIndex]== '\n') {
                        if (prevChar=='\r') {
                            //there is content type + \r\n then \r\n again
                            //Note may be need to check
                            if (contentTypePassed) {
                                parsing_state = parse_file_content;
                                contentTypePassed = false;
                                //TODO:
                                //Check if have filename size in args
                                //if yes update filesize accordingly
                                //TODO: if path and path not visible in filenameValue
                                //re-generate filenameValue with path
                                if (post_upload_ctx->writeFn) {
                                    if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_start,  fileDescriptor, filenameValue.c_str(), fileSize)) {
                                        esp3d_log_e("Error writing file invalid");
                                        return ESP_FAIL;
                                    }
                                }
                                esp3d_log("Data from Content-Type section parsed");
                            } else {
                                esp3d_log("Content-Type section parsed: %s",contentBuffer.c_str());
                                contentTypePassed=true;
                                contentBuffer="";
                            }
                        } else {
                            esp3d_log_e("Data from Content-Type section is invalid");
                            return ESP_FAIL;
                        }
                    } else {
#if ESP3D_TFT_LOG
                        if (!contentTypePassed && packet[pIndex]!= '\r') {
                            contentBuffer+=packet[pIndex];
                        }
#endif //ESP3D_TFT_LOG
                    }
                    break;
                case parse_file_content:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (indexPacketWrite==PACKET_WRITE_SIZE) {
                        //TODO: send to write function
                        if (post_upload_ctx->writeFn) {
                            if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)packetWrite, indexPacketWrite, upload_file_write,  fileDescriptor, filenameValue.c_str(), fileSize)) {
                                esp3d_log_e("Error writing file invalid");
                                return ESP_FAIL;
                            }
                        }
                        indexPacketWrite=0;
                    }
                    packetWrite[indexPacketWrite]=packet[pIndex];
                    indexPacketWrite++;
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
        return ESP_FAIL;
    }
    if (remaining == 0) {
        // it is last boundary we assume the 4 last chars are `--\r\n`
        // at this stage if not the case it is not really an issue anymore
        esp3d_log("Now go to new request handle");
        return (post_upload_ctx->nextHandler(req));
    }
    esp3d_log_e("Error should not be there %d", remaining);
    return ESP_FAIL;
}
