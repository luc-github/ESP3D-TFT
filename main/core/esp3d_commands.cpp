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
    if (!msg) {
        return;
    }
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


bool Esp3DCommands::dispatchSetting(bool json,const char * filter, esp3d_setting_index_t index, const char* help, const char ** optionValues, const char ** optionLabels,  uint32_t maxsize, uint32_t minsize, uint32_t minsize2,uint8_t precision, const char * unit, bool needRestart,esp3d_clients_t target, esp3d_request_t requestId, bool isFirst)
{
    std::string tmpstr;
    std::string value;
    char out_str[255];
    tmpstr.reserve(350); //to save time and avoid several memories allocation delay
    const Esp3DSetting_t * elementSetting = esp3dTFTsettings.getSettingPtr(index);
    if (!elementSetting) {
        return false;
    }
    switch(elementSetting->type) {
    case  esp3d_byte:
        value= std::to_string(esp3dTFTsettings.readByte(index));
        break;
    case esp3d_integer:
        value=std::to_string(esp3dTFTsettings.readUint32(index));
        break;
    case esp3d_ip:
        value=esp3dTFTsettings.readIPString(index);
        break;
    case esp3d_float:
        //TODO Add float support ?
        value="Not supported";
        break;
    case esp3d_mask:
        //TODO Add Mask support ?
        value="Not supported";
        break;
    case esp3d_bitsfield:
        //TODO Add bitfield support ?
        value="Not supported";
        break;
    default: //String
        if (index==esp3d_sta_password || index==esp3d_ap_password) { //hide passwords using  ********
            value=HIDDEN_SETTING_VALUE;
        } else {
            value=esp3dTFTsettings.readString(index, out_str, elementSetting->size);
        }
    }
    if (json) {
        if (!isFirst) {
            tmpstr+=",";
        }
        tmpstr += "{\"F\":\"";
        tmpstr+=filter;
        tmpstr += "\",\"P\":\"";
        tmpstr+=std::to_string(index);
        tmpstr += "\",\"T\":\"";
        switch(elementSetting->type) {
        case  esp3d_byte:
            tmpstr+="B";
            break;
        case esp3d_integer:
            tmpstr+="I";
            break;
        case esp3d_ip:
            tmpstr+="A";
            break;
        case esp3d_float:
            tmpstr+="B";
            break;
        case esp3d_mask :
            tmpstr+="M";
            break;
        case esp3d_bitsfield:
            tmpstr+="X";
            break;
        default:
            tmpstr+="S";
        }
        tmpstr += "\",\"V\":\"";
        tmpstr += value;
        tmpstr += "\",\"H\":\"";
        tmpstr += help;
        tmpstr += "\"";
        if (needRestart) {
            tmpstr += ",\"R\":\"1\"";
        }
        if (optionValues && optionLabels) {
            tmpstr += ",\"O\":[";
            for (uint8_t i =0; i<maxsize; i++) {
                if (i>0) {
                    tmpstr += ",";
                }
                tmpstr += "{\"";
                //be sure we have same size for both array to avoid overflow
                tmpstr += optionLabels[i];
                tmpstr += "\":\"";
                tmpstr += optionValues[i];
                tmpstr += "\"}";
            }
            tmpstr += "]";
        }
        if (unit) {
            tmpstr += ",\"R\":\"";
            tmpstr += unit;
            tmpstr +="\"";
        }
        if (precision!=((uint8_t)-1)) {
            tmpstr += ",\"E\":\"";
            tmpstr += std::to_string(precision);
            tmpstr +="\"";
        }
        if (maxsize!=(uint32_t)-1 && !optionValues) {
            tmpstr += ",\"S\":\"";
            tmpstr += std::to_string(maxsize);
            tmpstr +="\"";
        }
        if (minsize!=(uint32_t)-1) {
            tmpstr += ",\"M\":\"";
            tmpstr += std::to_string(minsize);
            tmpstr +="\"";
        }
        if (minsize2!=(uint32_t)-1) {
            tmpstr += ",\"MS\":\"";
            tmpstr += std::to_string(minsize2);
            tmpstr +="\"";
        }
        tmpstr += "}";
    } else {
        tmpstr=filter;
        tmpstr+="/";
        tmpstr+=help;
        tmpstr+=": ";
        tmpstr+=value;
        tmpstr+="\n";
    }
    return dispatch(tmpstr.c_str(), target, requestId);
}

bool Esp3DCommands::dispatchAuthenticationError(esp3d_msg_t * msg, uint cmdid, bool json)
{
    std::string tmpstr;
    if (!msg) {
        return false;
    }
    msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
    if (json) {
        tmpstr = "{\"cmd\":\"";
        tmpstr = std::to_string(cmdid);
        tmpstr+= "\",\"status\":\"error\",\"data\":\"Wrong authentication level\"}";
    } else {
        tmpstr = "Wrong authentication level\n";
    }
    return dispatch(msg,tmpstr.c_str());
}

bool  Esp3DCommands::dispatchAnswer(esp3d_msg_t * msg, uint cmdid, bool json, bool hasError, const char* answerMsg)
{
    std::string tmpstr;
    if (!msg) {
        esp3d_log_e("no msg");
        return false;
    }
    if (json) {
        tmpstr = "{\"cmd\":\""+std::to_string(cmdid) + "\",\"status\":\"";
        if (hasError) {
            tmpstr +="error";
        } else {
            tmpstr +="ok";
        }
        tmpstr+="\",\"data\":\"";
        tmpstr += answerMsg;
        tmpstr += "\"}";
    } else {
        tmpstr=answerMsg;
        tmpstr += "\n";
    }
    return dispatch(msg,tmpstr.c_str());
}

bool  Esp3DCommands::dispatchIdValue(bool json,const char *Id, const char * value, esp3d_clients_t target, esp3d_request_t requestId, bool isFirst)
{
    std::string tmpstr="";
    if (json) {
        if (!isFirst) {
            tmpstr+=",";
        }
        tmpstr += "{\"id\":\"";
    }
    tmpstr +=Id;
    if (json) {
        tmpstr += "\",\"value\":\"";
    } else {
        tmpstr += ": ";
    }
    tmpstr +=value;
    if (json) {
        tmpstr += "\"}";
    } else {
        tmpstr +="\n";
    }
    return dispatch(tmpstr.c_str(), target, requestId);
}


bool  Esp3DCommands::dispatch(const char * sbuf,  esp3d_clients_t target, esp3d_request_t requestId, esp3d_clients_t origin, esp3d_authentication_level_t authentication_level)
{
    esp3d_msg_t * newMsgPtr = Esp3DClient::newMsg( origin, target);
    if (newMsgPtr) {
        newMsgPtr->requestId = requestId;
        return dispatch(newMsgPtr,sbuf);
    }
    esp3d_log_e("no newMsgPtr");
    return false;
}

bool Esp3DCommands::dispatch(esp3d_msg_t * msg,const char * sbuf)
{
    return dispatch(msg,(uint8_t *) sbuf, strlen(sbuf));
}

bool  Esp3DCommands::dispatch(esp3d_msg_t * msg,uint8_t * sbuf, size_t len)
{
    if (!msg) {
        esp3d_log_e("no msg");
        return false;
    }
    if(!Esp3DClient::setDataContent (msg,sbuf, len)) {
        esp3d_log_e("Out of memory");
        Esp3DClient::deleteMsg(msg);
        return false;
    }
    return dispatch(msg);
}

bool Esp3DCommands::dispatch(esp3d_msg_t * msg)
{
    bool sendOk = true;
    if (!msg) {
        esp3d_log_e("no msg");
        return false;
    }
    //currently only echo back no test done on success
    //TODO check add is successful
    switch (msg->target) {
    case SERIAL_CLIENT:
        if (serialClient.started()) {
            if (!serialClient.addTXData(msg)) {
                serialClient.flush();
                if (!serialClient.addTXData(msg)) {
                    esp3d_log_e("Cannot add msg to client queue");
                    sendOk=false;
                }
            } else {
                serialClient.flush();
            }
        }
        break;
    case ALL_CLIENTS:
        //TODO: msg need to be duplicate for each target
        if (msg->origin!=SERIAL_CLIENT) {
            if (serialClient.started()) {
                if (!serialClient.addTXData(msg)) {
                    esp3d_log_e("Cannot add msg to client queue");
                    sendOk=false;
                }
            }
        }
        break;
    default:
        esp3d_log_e("No valid target specified %d", msg->target);
        sendOk = false;
    }
    //clear message
    if (!sendOk) {
        esp3d_log_e("Send msg failed");
        Esp3DClient::deleteMsg(msg);
    }
    return sendOk;
}

bool Esp3DCommands::hasTag (esp3d_msg_t * msg, uint start,const char* label)
{
    if (!msg) {
        esp3d_log_e("no msg for tag %s", label);
        return false;
    }
    std::string lbl=label;
    esp3d_log("checking message for tag %s", label);
    uint lenLabel = strlen(label);
    lbl+="=";
    lbl = get_param (msg, start,lbl.c_str());
    if (lbl.length()!=0) {
        esp3d_log("Label is used with parameter %s", lbl.c_str());
        //make result uppercase
        str_toUpperCase(&lbl);
        return (lbl=="YES" || lbl=="1" || lbl=="TRUE");
    }
    bool prevCharIsEscaped = false;
    bool prevCharIsspace= true;
    esp3d_log("Checking  label as tag");
    for (uint i = start; i <msg->size; i++) {
        char c = char(msg->data[i]);
        esp3d_log("%c", c);
        if (c== label[0] && prevCharIsspace) {
            uint p = 0;
            while (i<msg->size && p <lenLabel && c==label[p] ) {
                i++;
                p++;
                if(i<msg->size) {
                    c = char(msg->data[i]);
                    esp3d_log("%c vs %c", c, char(msg->data[i]));
                }
            }
            if (p == lenLabel) {
                //end of params
                if (i==msg->size || std::isspace(c)) {
                    esp3d_log("label %s found", label);
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
    esp3d_log("label %s not found", label);
    return false;
}

const char *  Esp3DCommands::get_param (esp3d_msg_t * msg, uint start,const char* label)
{
    if (!msg) {
        return "";
    }
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
    if (!msg) {
        return "";
    }
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

bool Esp3DCommands::has_param (esp3d_msg_t * msg, uint start)
{
    return strlen(get_clean_param (msg,start))!=0;
}

void Esp3DCommands::execute_internal_command(int cmd, int cmd_params_pos,esp3d_msg_t * msg)
{
    // execute commands
    if (!msg) {
        return;
    }
#if ESP3D_AUTHENTICATION_FEATURE
    std::string pwd =  get_param (msg,cmd_params_pos,"pwd=");
    if (!pwd.empty()) { //adjust authentication level according
        msg->authentication_level=  esp3dAuthenthicationService.getAuthenticatedLevel(pwd.c_str());
    }
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
    if (msg->origin ==SERIAL_CLIENT) {
        msg->authentication_level=ESP3D_LEVEL_ADMIN;
    }
#endif //ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
#endif //ESP3D_AUTHENTICATION_FEATURE

    switch (cmd) {
    case 0:
        ESP0(cmd_params_pos, msg);
        break;
    case 200:
        ESP200(cmd_params_pos, msg);
        break;
    case 202:
        ESP202(cmd_params_pos, msg);
        break;
    case 400:
        ESP400(cmd_params_pos, msg);
        break;
    case 401:
        ESP401(cmd_params_pos, msg);
        break;
    case 420:
        ESP420(cmd_params_pos, msg);
        break;
    case 444:
        ESP444(cmd_params_pos, msg);
        break;
    case 710:
        ESP710(cmd_params_pos, msg);
        break;
    case 720:
        ESP720(cmd_params_pos, msg);
        break;
    case 730:
        ESP730(cmd_params_pos, msg);
        break;
    case 740:
        ESP740(cmd_params_pos, msg);
        break;
    case 750:
        ESP750(cmd_params_pos, msg);
        break;
    case 780:
        ESP780(cmd_params_pos, msg);
        break;
    case 790:
        ESP790(cmd_params_pos, msg);
        break;
    case 900:
        ESP900(cmd_params_pos, msg);
        break;
    case 901:
        ESP901(cmd_params_pos, msg);
        break;
    default:
        msg->target = msg->origin;
        if (hasTag(msg,cmd_params_pos,"json")) {
            if(!dispatch(msg, "{\"cmd\":\"0\",\"status\":\"error\",\"data\":\"Invalid Command\"}")) {
                esp3d_log_e("Out of memory");
            }
        } else {
            if(!dispatch(msg, "Invalid Command\n")) {
                esp3d_log_e("Out of memory");
            }
        }
    }
}

void Esp3DCommands::flush()
{
    serialClient.flush();
}