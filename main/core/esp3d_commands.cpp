/*
  esp3d_commands class
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

#include "esp3d_commands.h"
#include "esp3d_client.h"
#include "serial/esp3d_serial_client.h"
#include "esp3d_string.h"
#include <stdio.h>
#include <string>
Esp3DCommands esp3dCommands;

Esp3DCommands::Esp3DCommands() {}
Esp3DCommands::~Esp3DCommands() {}
bool Esp3DCommands::is_esp_command(uint8_t * sbuf, size_t len)
{
    if (len < 5) {
        return false;
    }
    //normal command header
    if ((char(sbuf[0]) == '[') && (char(sbuf[1]) == 'E') && (char(sbuf[2]) == 'S') && (char(sbuf[3]) == 'P') && ((char(sbuf[4]) == ']') ||(char(sbuf[5]) == ']')||(char(sbuf[6]) == ']') ||(char(sbuf[7]) == ']'))) {
        return true;
    }
    //echo command header on some targeted firmware
    if (len >= 14) {
        if((char(sbuf[0]) == 'e') && (char(sbuf[1]) == 'c') && (char(sbuf[2]) == 'h') && (char(sbuf[3]) == 'o') && (char(sbuf[4]) == ':') && (char(sbuf[5]) == ' ') && (char(sbuf[6]) == '[') && (char(sbuf[7]) == 'E')&& (char(sbuf[8]) == 'S') && (char(sbuf[9]) == 'P') && ((char(sbuf[4]) == ']') ||(char(sbuf[5]) == ']')||(char(sbuf[6]) == ']') ||(char(sbuf[7]) == ']'))) {
            return true;
        }
    }
    return false;
}

void Esp3DCommands::process(esp3d_msg_t * msg)
{
    static bool lastIsESP3D = false;
    if (is_esp_command(msg->data, msg->size)) {
        esp3d_log("Detected ESP command");
        lastIsESP3D = true;
        uint cmdId = 0;
        uint espcmdpos=0;
        bool startcmd = false;
        bool endcmd = false;
        for (uint i = 0; i < msg->size && espcmdpos==0; i++) {
            if (char(msg->data[i])==']') { //start command flag
                endcmd = true;
                espcmdpos=i+1;
            } else if(char(msg->data[i])=='[') { //end command flag
                startcmd = true;
            } else if (startcmd && !endcmd && std::isdigit(static_cast<unsigned char>(char(msg->data[i])))) { //command id
                if(cmdId!=0) {
                    cmdId = (cmdId*10);
                }
                cmdId+=(msg->data[i]-48);
            }
        }
        //execute esp command
        execute_internal_command(cmdId, espcmdpos, msg);
    } else {
        esp3d_log("Dispatch command");
        //Work around to avoid to dispatch single \n or \r to everyone as it is part of previous ESP3D command
        if (msg->size == 1 && ((char(msg->data[0])=='\n') || (char(msg->data[0])=='\r'))&& lastIsESP3D) {
            lastIsESP3D = false;
            //delete message
            Esp3DClient::deleteMsg(msg);
            return;
        }
        lastIsESP3D = false;
        dispatch(msg);
    }
}

bool Esp3DCommands::dispatch(esp3d_msg_t * msg,const char * sbuf)
{
    return dispatch(msg,(uint8_t *) sbuf, strlen(sbuf));
}

bool  Esp3DCommands::dispatch(esp3d_msg_t * msg,uint8_t * sbuf, size_t len)
{
    if(!Esp3DClient::setDataContent (msg,sbuf, len)) {
        esp3d_log_e("Out of memory");
        return false;
    }
    return dispatch(msg);
}

bool Esp3DCommands::dispatch(esp3d_msg_t * msg)
{
    bool sendOk = true;
    //currently only echo back no test done on success
    //TODO check add is successful
    switch (msg->target) {
    case SERIAL_CLIENT:
        if (!serialClient.addTXData(msg)) {
            sendOk=false;
        }
        break;
    case ALL_CLIENTS:
        //TODO: msg need to be duplicate for each target
        if (msg->origin!=SERIAL_CLIENT) {
            if (!serialClient.addTXData(msg)) {
                sendOk=false;
            }
        }
        break;
    default:
        esp3d_log("No valid target specified %d", msg->target);
        sendOk = false;
    }
    //clear message
    if (!sendOk) {
        Esp3DClient::deleteMsg(msg);
    }
    return sendOk;
}

bool Esp3DCommands::hasTag (esp3d_msg_t * msg, uint start,const char* label)
{
    std::string lbl=label;
    uint lenLabel = strlen(label);
    lbl+="=";
    lbl = get_param (msg, start,lbl.c_str());
    if (lbl.length()!=0) {
        //make result uppercase
        str_toUpperCase(&lbl);
        return (lbl=="YES" || lbl=="1" || lbl=="TRUE");
    }
    bool prevCharIsEscaped = false;
    bool prevCharIsspace= true;
    for (uint i = start; i <msg->size; i++) {
        char c = char(msg->data[i]);
        if (c== label[0] && prevCharIsspace) {
            uint p = 0;
            while (i<msg->size && p <lenLabel && c==label[p] ) {
                i++;
                p++;
                if(i<msg->size) {
                    c = char(msg->data[i]);
                }
            }
            if (p == lenLabel) {
                //end of params
                if (i==msg->size) {
                    return true;
                }
                //next char is space
                if (std::isspace(c)) {
                    return true;
                }
            }
            if (std::isspace(c) && !prevCharIsEscaped) {
                prevCharIsspace=true;
            }
            if (c=='\\') {
                prevCharIsEscaped=true;
            } else {
                prevCharIsEscaped=false;
            }

        }
    }
    return false;
}

const char *  Esp3DCommands::get_param (esp3d_msg_t * msg, uint start,const char* label)
{
    int startPos = -1;
    uint lenLabel = strlen(label);
    static std::string value;
    bool prevCharIsEscaped = false;
    bool prevCharIsspace= true;
    value.clear();
    uint startp = start;
    while( char(msg->data[startp])==' ' && startp<msg->size) {
        startp++;
    }
    for (uint i = startp; i <msg->size; i++) {
        char c = char(msg->data[i]);
        if (c== label[0] && startPos == -1 && prevCharIsspace) {
            uint p = 0;
            while (i<msg->size && p <lenLabel && c==label[p] ) {
                i++;
                p++;
                if(i<msg->size) {
                    c = char(msg->data[i]);
                }
            }
            if (p == lenLabel) {
                startPos = i;
            }
        }
        if (std::isspace(c) && !prevCharIsEscaped) {
            prevCharIsspace=true;
        }
        if (startPos>-1 && i<msg->size) {
            if (c=='\\') {
                prevCharIsEscaped=true;
            }
            if (std::isspace(c) && !prevCharIsEscaped ) {
                return value.c_str();
            }

            if (c!='\\') {
                value.append(1,c);
                prevCharIsEscaped=false;
            }
        }
    }
    return value.c_str();
}

const char * Esp3DCommands::get_clean_param (esp3d_msg_t * msg, uint start)
{
    static std::string value;
    bool prevCharIsEscaped = false;
    uint startp = start;
    while( char(msg->data[startp])==' ' && startp<msg->size) {
        startp++;
    }
    value.clear();
    for (uint i = startp; i <msg->size; i++) {
        char c = char(msg->data[i]);
        if (c=='\\') {
            prevCharIsEscaped=true;
        }
        if (std::isspace(c) && !prevCharIsEscaped ) {
            if (value=="json" ||value.find("json=")==0 || value.find("pwd=")==0) {
                value.clear();
            } else {
                return value.c_str();
            }
        }
        if (c!='\\') {
            if ((std::isspace(c) && prevCharIsEscaped) || !std::isspace(c)) {
                value.append(1,c);
            }
            prevCharIsEscaped=false;
        }
    }
    return value.c_str();
}

void Esp3DCommands::execute_internal_command(int cmd, int cmd_params_pos,esp3d_msg_t * msg)
{
    // execute commands
    switch (cmd) {
    case 0:
        ESP0(cmd_params_pos, msg);
        break;
    case 420:
        ESP420(cmd_params_pos, msg);
        break;
    default:
        msg->target = msg->origin;
        if(!dispatch(msg, "Invalid Command")) {
            Esp3DClient::deleteMsg(msg);
            esp3d_log_e("Out of memory");
        }
    }

}
