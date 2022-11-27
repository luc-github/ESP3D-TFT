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
    parse_content_disposition,
    parse_content_type,
    parse_content_separator,
    parse_data_form,
    parse_data_file
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
    uint8_t boundaryCursor = 2;

    uint indexPacketWrite=0;
    std::string boundaryString = "\r\n--";
    std::string contentBuffer;
    char prevChar=0x0;
    std::string argName = "";
    std::string argValue = "";
    std::string fileName = "";
    esp3d_upload_state_t uploadState = upload_file_start;
    boundaryString += boundaryPtr;
    boundaryString += "\r\n";
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
                if (parsing_state==parse_data_file && post_upload_ctx->writeFn) {
                    if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_aborted,  fileDescriptor, fileName.c_str(), fileSize)) {
                        esp3d_log_e("Error writing file invalid");
                    }
                }
                return ESP_FAIL;
            }
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                esp3d_log_e("Time out");
                continue;
            }
            if (received == HTTPD_SOCK_ERR_INVALID || received == HTTPD_SOCK_ERR_FAIL) {
                esp3d_log_e("Error connection");
                if (parsing_state==parse_data_file && post_upload_ctx->writeFn) {
                    if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_aborted,  fileDescriptor, fileName.c_str(), fileSize)) {
                        esp3d_log_e("Error writing file invalid");
                    }
                }
                return ESP_FAIL;
            }
            //decrease received bytes from remaining bytes amount
            remaining -= received;
            //Parsing the buffer
            for (uint pIndex = 0; pIndex < received; pIndex++) {
                // parse buffer
                switch (parsing_state) {
                //--XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                case parse_boundary:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (boundaryCursor < boundaryString.length()) {
                        if (boundaryString[boundaryCursor] == packet[pIndex]) {
                            boundaryCursor++;
                            if (boundaryCursor == boundaryString.length()) {
                                parsing_state = parse_content_disposition;
                            }
                        } else {
                            if (remaining >= 4) {
                                esp3d_log_e("Error parsing boundary");
                                return ESP_FAIL;
                            } else {
                                if (packet[pIndex]== '-' || packet[pIndex]== '\r' || packet[pIndex]== '\n') {
                                    esp3d_log("End of boundary body");
                                } else {
                                    esp3d_log_e("Error parsing end boundary remaining: %d", remaining);
                                    return ESP_FAIL;
                                }

                            }
                        }
                    } else {
                        esp3d_log_e("Error parsing end of boundary");
                        return ESP_FAIL;
                    }
                    break;
                case parse_content_disposition:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (packet[pIndex] == '\n') {
                        if (prevChar == '\r') {
                            esp3d_log("Got %s", contentBuffer.c_str());
                            if (!esp3d_strings::startsWith(contentBuffer.c_str(),"Content-Disposition: form-data; ")) {
                                esp3d_log_e("Error parsing content disposition,missing content-disposition header");
                                return ESP_FAIL;
                            }
                            //check name parameter
                            int startPos = esp3d_strings::find(contentBuffer.c_str(),"name=");
                            if (startPos == -1) {
                                esp3d_log_e("Error parsing content disposition,missing name parameter");
                                return ESP_FAIL;
                            }
                            startPos+=6; //size of name="
                            int endPos = esp3d_strings::find(contentBuffer.c_str(),"\"", startPos);
                            if (endPos == -1) {
                                esp3d_log_e("Error parsing content disposition,missing name parameter");
                                return ESP_FAIL;
                            }
                            argName = contentBuffer.substr(startPos, endPos-startPos);
                            esp3d_log("Got name=%s",argName.c_str());

                            //check fileName parameter
                            startPos = esp3d_strings::find(contentBuffer.c_str(),"filename=");
                            if (startPos != -1) {
                                startPos+=10; //size of filename="
                                int endPos = esp3d_strings::find(contentBuffer.c_str(),"\"", startPos);
                                if (endPos == -1) {
                                    esp3d_log_e("Error parsing content disposition,missing name parameter");
                                    return ESP_FAIL;
                                }
                                fileName = contentBuffer.substr(startPos, endPos-startPos);
                                esp3d_log("Got filename=%s",fileName.c_str());
                            }

                            parsing_state = parse_content_type;
                            contentBuffer = "";
                        } else {
                            esp3d_log_e("Error parsing content disposition, wrong end of line");
                            return ESP_FAIL;
                        }
                    } else {
                        if (contentBuffer.length()>300) {
                            esp3d_log_e("Error parsing content disposition, wrong size");
                            return ESP_FAIL;
                        } else {
                            contentBuffer+=packet[pIndex];
                        }
                    }
                    break;
                case parse_content_type:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (packet[pIndex] == '\n') {
                        if (prevChar == '\r') {
                            esp3d_log("Got %s", contentBuffer.c_str());
                            esp3d_log("size: %d", contentBuffer.length());
                            if (contentBuffer.length()==0) {
                                if (fileName.length()>0) {
                                    parsing_state = parse_data_file;
                                } else {
                                    parsing_state = parse_data_form;
                                    uploadState = upload_file_start;
                                }
                            } else {
                                parsing_state = parse_content_separator;
                            }
                            contentBuffer="";
                        } else {
                            esp3d_log_e("Error parsing content disposition, wrong end of line");
                            return ESP_FAIL;
                        }
                    } else {
                        if (contentBuffer.length()<255 ) {
                            if(packet[pIndex]!='\r') {
                                contentBuffer+=packet[pIndex];
                            }
                        } else {
                            esp3d_log_e("Error parsing content type, wrong size");
                            return ESP_FAIL;
                        }
                    }

                    break;
                case parse_content_separator:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (packet[pIndex] == '\n' || packet[pIndex] == '\r') {
                        if (packet[pIndex] == '\n' && prevChar == '\r') {
                            if (fileName.length()>0) {
                                parsing_state = parse_data_file;
                            } else {
                                parsing_state = parse_data_form;
                            }
                        }
                    } else {
                        esp3d_log_e("Error parsing content disposition, wrong end of line");
                        return ESP_FAIL;
                    }
                    break;
                case parse_data_form:
                    //esp3d_log("Parsing %d:  char %d:%c", pIndex, packet[pIndex], packet[pIndex]);
                    if (packet[pIndex] == '\n') {
                        if (prevChar == '\r') {
                            esp3d_log("Got %s=%s", argName.c_str(), argValue.c_str());
                            post_upload_ctx->args.push_back(std::make_pair(argName,argValue));
                            argName="";
                            argValue="";
                            parsing_state = parse_boundary;
                            boundaryCursor = 2;
                        } else {
                            esp3d_log_e("Error parsing data form, wrong end of line");
                            return ESP_FAIL;
                        }
                    } else {
                        if (argValue.length() < 255) {
                            if(packet[pIndex] != '\r' && argValue.length() < 255) {
                                argValue+=packet[pIndex] ;
                            }
                        } else {
                            esp3d_log_e("Error parsing content type, wrong size");
                            return ESP_FAIL;
                        }
                    }
                    break;
                case parse_data_file:
                    if (uploadState == upload_file_start) {
                        //if path and path not visible in filenameValue
                        //re-generate filenameValue with path
                        if (esp3dHttpService.hasArg(req,"path")) {
                            std::string path = esp3dHttpService.getArg(req,"path");
                            esp3d_log("Path from post: %s", path.c_str());
                            if (!esp3d_strings::startsWith(fileName.c_str(),path.c_str())) {
                                if (path[path.length()-1] != '/') {
                                    path+="/";
                                }
                                fileName=path + fileName;
                            }
                        }
                        //Check if have filename size in args
                        //if yes update filesize accordingly
                        std::string fileSizeArg=fileName+"S";
                        if (esp3dHttpService.hasArg(req,fileSizeArg.c_str())) {
                            fileSize = atoi(esp3dHttpService.getArg(req,fileSizeArg.c_str()));
                            esp3d_log("File size from post: %d", fileSize);
                        }
                        if (post_upload_ctx->writeFn) {
                            if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_start,  fileDescriptor, fileName.c_str(), fileSize)) {
                                esp3d_log_e("Error writing file invalid");
                                return ESP_FAIL;
                            }
                        }
                        uploadState = upload_file_write;
                        boundaryCursor=0;
                    }

                    if (packet[pIndex]==boundaryString[boundaryCursor]) {
                        boundaryCursor++;
                        //this is boundary string but not final one
                        if (boundaryCursor==boundaryString.length()) {
                            uploadState = upload_file_end;
                            boundaryCursor= 0;
                            parsing_state = parse_content_disposition;
                        }
                    } else {
                        if (boundaryCursor==0) {
                            packetWrite[indexPacketWrite]=packet[pIndex];
                            indexPacketWrite++;
                        } else {
                            if (packet[pIndex]=='-' && remaining<4) {
                                uploadState = upload_file_end;
                                parsing_state = parse_boundary;
                            } else {
                                //the data looks like begining of boundary but it is not finalized
                                //so copy boundary part to write buffer
                                //can be /r/n of the file content and any additional data identical to boundary start part
                                for (uint c= 0; c<boundaryCursor; c++) {
                                    packetWrite[indexPacketWrite]=boundaryString[c];
                                    indexPacketWrite++;
                                    if (indexPacketWrite==PACKET_WRITE_SIZE) {
                                        if (post_upload_ctx->writeFn) {
                                            if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)packetWrite, indexPacketWrite, upload_file_write,  fileDescriptor, fileName.c_str(), fileSize)) {
                                                esp3d_log_e("Error writing file invalid");
                                                return ESP_FAIL;
                                            }
                                        }
                                        indexPacketWrite=0;
                                    }
                                }
                                //reprocess current char with new context
                                boundaryCursor=0;
                                pIndex--;
                                continue;
                            }
                        }

                    }
                    if (indexPacketWrite==PACKET_WRITE_SIZE || (uploadState == upload_file_end && indexPacketWrite>0)) {
                        if (post_upload_ctx->writeFn) {
                            if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)packetWrite, indexPacketWrite, upload_file_write,  fileDescriptor, fileName.c_str(), fileSize)) {
                                esp3d_log_e("Error writing file invalid");
                                return ESP_FAIL;
                            }
                        }
                        indexPacketWrite=0;
                    }

                    if (uploadState == upload_file_end) {
                        //send end of file
                        if (post_upload_ctx->writeFn) {
                            if (ESP_OK!=post_upload_ctx->writeFn((const uint8_t *)nullptr, 0, upload_file_end,  fileDescriptor, fileName.c_str(), fileSize)) {
                                esp3d_log_e("Error writing file invalid");
                                return ESP_FAIL;
                            }
                        }
                        uploadState = upload_file_start;
                        fileName="";
                        fileSize=-1;
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
