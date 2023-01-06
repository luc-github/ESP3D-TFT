/*
  esp3d_commands member
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
#include "esp3d_string.h"
#include "authentication/esp3d_authentication.h"
#include "sd_def.h"
#define COMMAND_ID 400
const char * BaudRateList[] = {"9600", "19200", "38400", "57600", "74880", "115200", "230400", "250000", "500000", "921600"};
const char * SPIDivider[] = { "1", "2", "4", "6", "8", "16", "32"};
const char * YesNoLabels [] = { "yes", "no"};
const char * YesNoValues [] = { "1", "0"};
const char * RadioModeLabels [] = { "none",
                                    "sta",
                                    "ap",
                                    "setup"
                                  };
const char * RadioModeValues [] = { "0",
                                    "1",
                                    "2",
                                    "3"
                                  };

const char * FirmwareLabels [] = { "Unknown",
                                   "Grbl",
                                   "Marlin",
                                   "Smoothieware",
                                   "Repetier",
                                   "grblHAL",
                                   "HP_GL"
                                 };

const char * FirmwareValues [] = { "0",
                                   "10",
                                   "20",
                                   "40",
                                   "50",
                                   "80",
                                   "90"
                                 };
const char * NotificationsLabels [] = { "none",
                                        "pushover",
                                        "email",
                                        "line",
                                        "telegram",
                                        "ifttt"
                                      };

const char * NotificationsValues [] = { "0",
                                        "1",
                                        "2",
                                        "3",
                                        "4",
                                        "5"
                                      };

const char * IpModeLabels [] = { "dhcp", "static"};
const char * IpModeValues [] = { "0", "1"};

const char * FallbackLabels [] = { "none", "setup"};
const char * FallbackValues [] = { "0", "3"};

const char * ApChannelsList [] = { "1", "2","3","4","5","6","7","8","9","10","11","12","13","14"};

#define MIN_PORT_NUMBER 1
#define MAX_PORT_NUMBER 65535




//Get full ESP3D settings
//[ESP400]<pwd=admin>
void Esp3DCommands::ESP400(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;

    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, COMMAND_ID, json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (json) {
        tmpstr = "{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[";

    } else {
        tmpstr = "Settings:\n";
    }
    msg->type = msg_head;
    if(!dispatch(msg,tmpstr.c_str())) {
        esp3d_log_e("Error sending response to clients");
        return;
    }
    //Baud rate (first item)
    if (!dispatchSetting(json,"system/system",esp3d_baud_rate, "baud", BaudRateList, BaudRateList, sizeof(BaudRateList)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId,true)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Target Firmware
    if (!dispatchSetting(json,"system/system",esp3d_target_firmware, "targetfw", FirmwareValues, FirmwareLabels, sizeof(FirmwareValues)/sizeof(char*), -1, -1,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Radio mode
    if (!dispatchSetting(json,"network/network",esp3d_radio_mode, "radio mode", RadioModeValues, RadioModeLabels, sizeof(RadioModeValues)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Radio boot mode
    if (!dispatchSetting(json,"network/network",esp3d_radio_mode, "radio_boot", YesNoValues, YesNoLabels, sizeof(YesNoValues)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Hostname
    if (!dispatchSetting(json,"network/network",esp3d_hostname, "hostname", nullptr, nullptr, SIZE_OF_SETTING_HOSTNAME, 1, 1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA SSID
    if (!dispatchSetting(json,"network/sta",esp3d_sta_ssid, "SSID", nullptr, nullptr, SIZE_OF_SETTING_SSID_ID, 1, 1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA Password
    if (!dispatchSetting(json,"network/sta",esp3d_sta_password, "pwd", nullptr, nullptr, SIZE_OF_SETTING_SSID_PWD, 8, 0,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA ip mode
    if (!dispatchSetting(json,"network/sta",esp3d_sta_ip_mode, "ip mode", IpModeValues, IpModeLabels, sizeof(IpModeLabels)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA fallback mode
    if (!dispatchSetting(json,"network/sta",esp3d_fallback_mode, "sta fallback mode", FallbackValues, FallbackLabels, sizeof(FallbackValues)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA static ip
    if (!dispatchSetting(json,"network/sta",esp3d_sta_ip_static, "ip", nullptr, nullptr, -1, -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA static mask
    if (!dispatchSetting(json,"network/sta",esp3d_sta_mask_static, "msk", nullptr, nullptr, -1, -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA static gateway
    if (!dispatchSetting(json,"network/sta",esp3d_sta_gw_static, "gw", nullptr, nullptr, -1, -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //STA static dns
    if (!dispatchSetting(json,"network/sta",esp3d_sta_dns_static, "DNS", nullptr, nullptr, -1, -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //AP SSID
    if (!dispatchSetting(json,"network/ap",esp3d_ap_ssid, "SSID", nullptr, nullptr, SIZE_OF_SETTING_SSID_ID, 1, 1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //AP Password
    if (!dispatchSetting(json,"network/ap",esp3d_ap_password, "pwd", nullptr, nullptr, SIZE_OF_SETTING_SSID_PWD, 8, 0,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //AP static ip
    if (!dispatchSetting(json,"network/ap",esp3d_ap_ip_static, "ip", nullptr, nullptr, -1, -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //AP Channel
    if (!dispatchSetting(json,"network/ap",esp3d_ap_channel, "channel", ApChannelsList, ApChannelsList, sizeof(ApChannelsList)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Http service on
    if (!dispatchSetting(json,"service/http",esp3d_http_on, "enable",YesNoValues, YesNoLabels, sizeof(YesNoValues)/sizeof(char*),-1,-1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Socket/Telnet port
    if (!dispatchSetting(json,"service/telnetd",esp3d_socket_port, "port", nullptr, nullptr,MAX_PORT_NUMBER, MIN_PORT_NUMBER, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Socket/Telnet
    if (!dispatchSetting(json,"service/telnetd",esp3d_socket_on, "enable",YesNoValues, YesNoLabels, sizeof(YesNoValues)/sizeof(char*),-1,-1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Http port
    if (!dispatchSetting(json,"service/http",esp3d_http_port, "port", nullptr, nullptr,MAX_PORT_NUMBER, MIN_PORT_NUMBER, -1,-1, nullptr, true,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }


    //Notifications type
    if (!dispatchSetting(json,"service/notification",esp3d_notification_type, "notification", NotificationsValues, NotificationsLabels,sizeof(NotificationsValues)/sizeof(char*), -1, -1,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Auto notifications
    if (!dispatchSetting(json,"service/notification",esp3d_auto_notification, "auto notif",YesNoValues, YesNoLabels, sizeof(YesNoValues)/sizeof(char*),-1,-1,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Notification token 1
    if (!dispatchSetting(json,"service/notification",esp3d_notification_token_1, "t1", nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_T1, 0, 0,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }
    //Notification token 2
    if (!dispatchSetting(json,"service/notification",esp3d_notification_token_2, "t2", nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_T2, 0, 0,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

    //Notification token setting
    if (!dispatchSetting(json,"service/notification",esp3d_notification_token_setting, "ts", nullptr, nullptr, SIZE_OF_SETTING_NOFIFICATION_TS, 0, 0,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }

#if defined(ESP3D_SD_IS_SPI) && ESP3D_SD_IS_SPI
    //SPI Divider factor
    if (!dispatchSetting(json,"device/sd",esp3d_spi_divider, "speedx", SPIDivider, SPIDivider,  sizeof(SPIDivider)/sizeof(char*), -1, -1,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }
#endif
    //SD updater
    if (!dispatchSetting(json,"device/sd",esp3d_check_update_on_sd, "SD updater", YesNoValues, YesNoLabels, sizeof(YesNoValues)/sizeof(char*), -1, -1,-1, nullptr, false,target,requestId)) {
        esp3d_log_e("Error sending response to clients");
    }


    if (json) {
        if(!dispatch("]}",target, requestId, msg_tail)) {
            esp3d_log_e("Error sending response to clients");
        }
    } else {
        if(!dispatch("ok\n",target, requestId, msg_tail)) {
            esp3d_log_e("Error sending response to clients");
        }
    }
}