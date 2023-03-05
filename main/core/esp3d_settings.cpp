
/*
  esp3d_settings.cpp -  settings esp3d functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with This code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "esp3d_settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "lwip/ip_addr.h"
#include "esp_system.h"
#include <cstring>
#include <regex>
#include <string>
#include "esp3d_log.h"
#include "esp3d_client.h"
#include "serial_def.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial_def.h"
#endif //ESP3D_USB_SERIAL_FEATURE
#include "network/esp3d_network.h"
#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif //ESP3D_NOTIFICATIONS_FEATURE

#include "authentication/esp3d_authentication.h"

#define STORAGE_NAME "ESP3D_TFT"
#define SETTING_VERSION "ESP3D_TFT-V3.0.1"

Esp3DSettings esp3dTFTsettings;

const uint32_t SupportedBaudList[] = {9600, 19200, 38400, 57600, 74880, 115200, 230400, 250000, 500000, 921600};
const uint8_t SupportedBaudListSize = sizeof(SupportedBaudList)/sizeof(uint32_t);

const uint8_t SupportedSPIDivider[] = { 1, 2, 4, 6, 8, 16, 32};
const uint8_t SupportedSPIDividerSize = sizeof(SupportedSPIDivider)/sizeof(uint8_t);

const uint8_t SupportedApChannels[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
const uint8_t SupportedApChannelsSize = sizeof(SupportedApChannels)/sizeof(uint8_t);


//value of settings, default value are all strings
const esp3d_setting_desc_t Esp3DSettingsData [] = {
    {esp3d_version, esp3d_string, SIZE_OF_SETTING_VERSION,"Invalid data"}, //Version
    {esp3d_baud_rate, esp3d_integer, 4, ESP3D_SERIAL_BAUDRATE},            //BaudRate
#if ESP3D_USB_SERIAL_FEATURE
    {esp3d_usb_serial_baud_rate, esp3d_integer, 4, ESP3D_USB_SERIAL_BAUDRATE}, //BaudRate
    {esp3d_output_client, esp3d_byte, 1,"1"},
#endif //ESP3D_USB_SERIAL_FEATURE
    {esp3d_hostname, esp3d_string, SIZE_OF_SETTING_HOSTNAME,"esp3d-tft"},
    {esp3d_radio_boot_mode, esp3d_byte, 1,"1"},
    {esp3d_radio_mode, esp3d_byte, 1,"3"},
    {esp3d_fallback_mode, esp3d_byte, 1,"3"},
    {esp3d_sta_ssid, esp3d_string, SIZE_OF_SETTING_SSID_ID,""},
    {esp3d_sta_password, esp3d_string, SIZE_OF_SETTING_SSID_PWD,""},
    {esp3d_sta_ip_mode, esp3d_byte, 1,"0"},
    {esp3d_sta_ip_static, esp3d_ip, 4,"192.168.1.100"},
    {esp3d_sta_mask_static, esp3d_ip, 4,"255.255.255.0"},
    {esp3d_sta_gw_static, esp3d_ip, 4,"192.168.1.1"},
    {esp3d_sta_dns_static, esp3d_ip, 4,"192.168.1.1"},
    {esp3d_ap_ssid, esp3d_string, SIZE_OF_SETTING_SSID_ID,"esp3dtft"},
    {esp3d_ap_password, esp3d_string, SIZE_OF_SETTING_SSID_PWD,"12345678"},
    {esp3d_ap_ip_static, esp3d_ip, 4,"192.168.0.1"},
    {esp3d_ap_channel, esp3d_byte, 1,"2"},
#if ESP3D_HTTP_FEATURE
    {esp3d_http_port, esp3d_integer, 4, "80"},
    {esp3d_http_on, esp3d_byte, 1,"1"},
#endif //ESP3D_HTTP_FEATURE
    {esp3d_setup, esp3d_byte, 1,"0"},
    {esp3d_target_firmware, esp3d_byte, 1,"0"},
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_FEATURE_IS_SPI
    {esp3d_spi_divider, esp3d_byte, 1, "1"}, //SPIdivider
#endif //ESP3D_SD_FEATURE_IS_SPI
#if ESP3D_UPDATE_FEATURE
    {esp3d_check_update_on_sd, esp3d_byte, 1,"1"},
#endif //ESP3D_UPDATE_FEATURE
#endif //ESP3D_SD_CARD_FEATURE

#if ESP3D_NOTIFICATIONS_FEATURE
    {esp3d_notification_type, esp3d_byte, 1,"0"},
    {esp3d_auto_notification, esp3d_byte, 1,"0"},
    {esp3d_notification_token_1,esp3d_string, SIZE_OF_SETTING_NOFIFICATION_T1,""},
    {esp3d_notification_token_2,esp3d_string, SIZE_OF_SETTING_NOFIFICATION_T2,""},
    {esp3d_notification_token_setting,esp3d_string, SIZE_OF_SETTING_NOFIFICATION_TS,""},
#endif //ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_TELNET_FEATURE
    {esp3d_socket_port, esp3d_integer, 4, "23"},
    {esp3d_socket_on, esp3d_byte, 1,"1"},
#endif //ESP3D_TELNET_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    {esp3d_ws_on, esp3d_byte, 1,"1"},
#endif //ESP3D_WS_SERVICE_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    {esp3d_admin_password, esp3d_string, SIZE_OF_LOCAL_PASSWORD,"admin"},
    {esp3d_user_password, esp3d_string, SIZE_OF_LOCAL_PASSWORD,"user"},
    {esp3d_session_timeout, esp3d_byte, 1,"3"},
#endif //ESP3D_AUTHENTICATION_FEATURE
};

bool  Esp3DSettings::isValidStringSetting(const char* value, esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * settingPtr= getSettingPtr(settingElement);
    if (!settingPtr) {
        return false;
    }
    if (!(settingPtr->type==esp3d_string)) {
        return false;
    }
    if (strlen(value)>settingPtr->size) {
        return false;
    }
    //use strlen because it crash with regex if value is longer than 41 characters
    size_t len = strlen(value);
    switch(settingElement) {
    case esp3d_ap_ssid:
    case esp3d_sta_ssid:
        return  (len>0 && len<=SIZE_OF_SETTING_SSID_ID); //any string from 1 to 32
    case esp3d_sta_password:
    case esp3d_ap_password:
        return (len==0 || (len>=8 && len<=SIZE_OF_SETTING_SSID_PWD)); //any string from 8 to 64 or 0
    case esp3d_hostname:
        esp3d_log("Checking hostname validity");
        return  std::regex_match (value, std::regex("^[a-zA-Z0-9]{1}[a-zA-Z0-9\\-]{0,31}$"));//any string alphanumeric or '-' from 1 to 32
#if ESP3D_NOTIFICATIONS_FEATURE
    case esp3d_notification_token_1:
        return len<=SIZE_OF_SETTING_NOFIFICATION_T1; //any string from 0 to 64
    case esp3d_notification_token_2:
        return len<=SIZE_OF_SETTING_NOFIFICATION_T2;  //any string from 0 to 64
    case esp3d_notification_token_setting:
        return len<=SIZE_OF_SETTING_NOFIFICATION_TS;  //any string from 0 to 128
#endif //ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
    case esp3d_admin_password:
    case esp3d_user_password:
        return len<=SIZE_OF_LOCAL_PASSWORD;  //any string from 0 to 20
#endif //ESP3D_AUTHENTICATION_FEATURE
    default:
        return false;
    }
    return false;
}

bool  Esp3DSettings::isValidIntegerSetting(uint32_t value, esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * settingPtr= getSettingPtr(settingElement);
    if (!settingPtr) {
        return false;
    }
    if (!(settingPtr->type==esp3d_integer || settingPtr->type==esp3d_ip)) {
        return false;
    }
    switch(settingElement) {
#if ESP3D_USB_SERIAL_FEATURE
    case esp3d_usb_serial_baud_rate:
#endif //#if ESP3D_USB_SERIAL_FEATURE
    case esp3d_baud_rate:
        for(uint8_t i=0; i<SupportedBaudListSize; i++) {
            if (SupportedBaudList[i]==value) {
                return true;
            }
        }
        break;
#if ESP3D_TELNET_FEATURE
    case esp3d_socket_port:
#endif //ESP3D_TELNET_FEATURE
#if ESP3D_HTTP_FEATURE
    case esp3d_http_port:
        if (value>=1 && value<65535) {
            return true;
        }
        break;
#endif //ESP3D_HTTP_FEATURE

    default:
        return false;
    }
    return false;
}

bool  Esp3DSettings::isValidByteSetting(uint8_t value, esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * settingPtr= getSettingPtr(settingElement);
    if (!settingPtr) {
        return false;
    }
    if (settingPtr->type!=esp3d_byte) {
        return false;
    }
    switch(settingElement) {
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
    case esp3d_check_update_on_sd:
#endif //ESP3D_UPDATE_FEATURE
#endif //ESP3D_SD_CARD_FEATURE

    case esp3d_setup:
#if ESP3D_TELNET_FEATURE
    case esp3d_socket_on:
#endif //ESP3D_TELNET_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    case esp3d_ws_on:
#endif //ESP3D_WS_SERVICE_FEATURE
#if ESP3D_HTTP_FEATURE
    case esp3d_http_on:
#endif //ESP3D_HTTP_FEATURE
    case esp3d_radio_boot_mode:
#if ESP3D_NOTIFICATIONS_FEATURE
    case esp3d_auto_notification:
#endif //ESP3D_NOTIFICATIONS_FEATURE
        if(value==(uint8_t)esp3d_state_off || value==(uint8_t)esp3d_state_on) {
            return true;
        }
        break;
#if ESP3D_AUTHENTICATION_FEATURE
    case esp3d_session_timeout:
        return true; //0 ->255 minutes
        break;
#endif //ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_USB_SERIAL_FEATURE
    case esp3d_output_client:
        return  ((esp3d_clients_t)value == SERIAL_CLIENT || (esp3d_clients_t)value == USB_SERIAL_CLIENT);
        break;
#endif //#if ESP3D_USB_SERIAL_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    case esp3d_notification_type:
        if(value==(uint8_t)esp3d_no_notification || value==(uint8_t)esp3d_pushover_notification || value==(uint8_t)esp3d_email_notification || value==(uint8_t)esp3d_line_notification || value==(uint8_t)esp3d_telegram_notification || value==(uint8_t)esp3d_ifttt_notification) {
            return true;
        }
        break;
#endif //ESP3D_NOTIFICATIONS_FEATURE

    case esp3d_sta_ip_mode:
        if(value==(uint8_t)esp3d_ip_mode_dhcp || value==(uint8_t)esp3d_ip_mode_static) {
            return true;
        }
        break;
    case esp3d_fallback_mode:
        if(value==(uint8_t)esp3d_radio_off  || value==(uint8_t)esp3d_wifi_ap_config || value==(uint8_t)esp3d_bluetooth_serial) {
            return true;
        }
        break;
    case esp3d_radio_mode:
        if (value==(uint8_t)esp3d_radio_off  || value==(uint8_t)esp3d_wifi_sta  || value==(uint8_t)esp3d_wifi_ap  || value==(uint8_t)esp3d_wifi_ap_config || value==(uint8_t)esp3d_bluetooth_serial) {
            return true;
        }
        break;
    case esp3d_ap_channel:
        for(uint8_t i=0; i<SupportedApChannelsSize; i++) {
            if (SupportedApChannels[i]==value) {
                return true;
            }
        }
        break;
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_FEATURE_IS_SPI
    case esp3d_spi_divider:
        for(uint8_t i=0; i<SupportedSPIDividerSize; i++) {
            if (SupportedSPIDivider[i]==value) {
                return true;
            }
        }
        break;
#endif//ESP3D_SD_FEATURE_IS_SPI
#endif //ESP3D_SD_CARD_FEATURE

    case esp3d_target_firmware:
        for (size_t i = esp3d_unknown; i !=last_esp3d_target_firmware_index_t; i++) {
            if ((esp3d_target_firmware_index_t)value == (esp3d_target_firmware_index_t)i) {
                return true;
            }
        }
        break;
    default:
        break;
    }
    return false;
}

bool Esp3DSettings::isValidIPStringSetting(const char* value, esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * settingPtr= getSettingPtr(settingElement);
    if (!settingPtr) {
        return false;
    }
    if (settingPtr->type!=esp3d_ip) {
        return false;
    }
    return std::regex_match (value, std::regex("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$"));
}

bool Esp3DSettings::isValidSettingsNvs()
{
    char result[SIZE_OF_SETTING_VERSION+1]  = {0};
    if (esp3dTFTsettings.readString(esp3d_version, result,SIZE_OF_SETTING_VERSION+1)) {
        if (strcmp(SETTING_VERSION, result)!=0) {
            esp3d_log_e("Expected %s but got %s", SETTING_VERSION, result);
            return false;
        } else {
            return true;
        }
    } else {
        esp3d_log_e("Cannot read setting version");
        return false;
    }
}

uint32_t Esp3DSettings::getDefaultIntegerSetting(esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * query = getSettingPtr(settingElement);
    if (query) {
        return (uint32_t)std::stoul(std::string(query->defaultval), NULL,0);
    }
    return 0;
}
const char * Esp3DSettings::getDefaultStringSetting(esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * query = getSettingPtr(settingElement);
    if (query) {
        return query->defaultval;
    }
    return nullptr;
}
uint8_t Esp3DSettings::getDefaultByteSetting(esp3d_setting_index_t settingElement)
{
    const esp3d_setting_desc_t * query = getSettingPtr(settingElement);
    if (query) {
        return (uint8_t)std::stoul(std::string(query->defaultval), NULL,0);
    }
    return 0;
}


const char * Esp3DSettings::GetFirmwareTargetShortName(esp3d_target_firmware_index_t index)
{
    switch(index) {
    case esp3d_grbl:
        return "grbl";
    case esp3d_marlin:
        return "marlin";
    case esp3d_marlin_embedded:
        return "marlin";
    case esp3d_smoothieware:
        return "smoothieware";
    case esp3d_repetier:
        return "repetier";
    case esp3d_reprap:
        return "reprap";
    case esp3d_grblhal:
        return "grblhal";
    case esp3d_hp_gl:
        return "hp_gl";
    default:
        break;
    }
    return "unknown";
}

bool Esp3DSettings::reset()
{
    esp3d_log("Resetting NVS");
    bool result = true;
    esp_err_t err;
    //clear all settings first
    nvs_handle_t handle_erase;
    err = nvs_open(STORAGE_NAME, NVS_READWRITE, &handle_erase);
    if (err != ESP_OK) {
        esp3d_log_e("Failing accessing NVS");
        return false;
    }
    if(nvs_erase_all(handle_erase)!= ESP_OK) {
        esp3d_log_e("Failing erasing NVS");
        return false;
    }
    nvs_close(handle_erase);

    //Init each setting with default value
    //this parsing method is to workaround parsing array of enums and usage of sizeof(<array of enums>) generate
    //warning: iteration 1 invokes undefined behavior [-Waggressive-loop-optimizations]
    //may be use a vector of enum would solve also
    //fortunaly this is to reset all settings so it is ok for current situation and it avoid to create an array of enums
    for (size_t i = esp3d_version; i !=last_esp3d_setting_index_t; i++) {
        esp3d_setting_index_t setting  =(esp3d_setting_index_t)i ;
        const esp3d_setting_desc_t * query = getSettingPtr(setting);
        if (query) {
            esp3d_log("Reseting %d value to %s",setting, query->defaultval);
            switch(query->type) {
            case  esp3d_byte:
                if (!Esp3DSettings::writeByte (setting, (uint8_t)std::stoul(std::string(query->defaultval), NULL,0) )) {
                    esp3d_log_e("Error writing %s to settings %d",query->defaultval, setting);
                    result = false;
                }
                break;
            case esp3d_ip:
                if (!Esp3DSettings::writeIPString (setting, query->defaultval) ) {
                    esp3d_log_e("Error writing %s to settings %d",query->defaultval, setting);
                    result = false;
                }
                break;
            case esp3d_integer:
                if (!Esp3DSettings::writeUint32 (setting, (uint32_t)std::stoul(std::string(query->defaultval), NULL,0) )) {
                    esp3d_log_e("Error writing %s to settings %d",query->defaultval, setting);
                    result = false;
                }
                break;
            case esp3d_string:
                if (setting==esp3d_version) {
                    if (!Esp3DSettings::writeString (setting, SETTING_VERSION)) {
                        esp3d_log_e("Error writing %s to settings %d",query->defaultval, setting);
                        result = false;
                    }
                } else {
                    if (!Esp3DSettings::writeString (setting, query->defaultval)) {
                        esp3d_log_e("Error writing %s to settings %d",query->defaultval, setting);
                        result = false;
                    }
                }

                break;
            default:
                result =false; //type is not handle
                esp3d_log_e("Setting  %d , type %d is not handled ",setting, query->type);
            }
        } else {
            result =false; //setting invalid
            esp3d_log_e("Setting  %d is unknown",setting);
        }
    }
    return result;
}

Esp3DSettings::Esp3DSettings()
{

}

Esp3DSettings::~Esp3DSettings()
{

}


uint8_t Esp3DSettings::readByte(esp3d_setting_index_t index, bool * haserror)
{
    uint8_t value = 0;
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_byte) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                esp3d_log_e("Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                err =handle->get_item(key.c_str(), value);
                if (err == ESP_OK) {
                    if (haserror) {
                        *haserror =false;
                    }
                    return value;
                }
                if (err == ESP_ERR_NVS_NOT_FOUND) {
                    value = (uint8_t)std::stoul(std::string(query->defaultval), NULL,0);
                    if (haserror) {
                        *haserror =false;
                    }
                    return value;
                }
            }
        }
    }
    if (haserror) {
        *haserror =true;
    }
    return 0;
}

uint32_t Esp3DSettings::readUint32(esp3d_setting_index_t index, bool * haserror)
{
    uint32_t value = 0;
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_integer || query->type== esp3d_ip) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
            if (err != ESP_OK) {
                esp3d_log_e( "Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                err = handle->get_item(key.c_str(), value);
                if (err == ESP_OK) {
                    if (haserror) {
                        *haserror =false;
                    }
                    return value;
                }
                if (err == ESP_ERR_NVS_NOT_FOUND) {
                    if(query->type== esp3d_integer) {
                        value = (uint32_t)std::stoul(std::string(query->defaultval), NULL,0);
                    } else { // ip is stored as uin32t but default value is ip format string
                        value = StringtoIPUInt32(query->defaultval);
                    }

                    if (haserror) {
                        *haserror =false;
                    }
                    return value;
                }
            }
        }
    }
    if (haserror) {
        *haserror =true;
    }
    return 0;
}

const char* Esp3DSettings::readIPString(esp3d_setting_index_t index, bool * haserror)
{
    uint32_t ipInt = readUint32(index,haserror);
    std::string ipStr = IPUInt32toString(ipInt);
    //esp3d_log("read setting %d : %d to %s",index, ipInt,ipStr.c_str());
    return IPUInt32toString(ipInt);
}

const char* Esp3DSettings::readString(esp3d_setting_index_t index, char* out_str, size_t len, bool * haserror)
{
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_string) {
            esp_err_t err;
            if (out_str && len >= (query->size)) {
                std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
                if (err != ESP_OK) {
                    esp3d_log_e("Failling accessing NVS");
                } else {
                    std::string key = "p_"+std::to_string((uint)query->index);
                    err = handle->get_string(key.c_str(), out_str, query->size);
                    if (err== ESP_OK) {
                        if (haserror) {
                            *haserror =false;
                        }
                        return out_str;
                    } else {
                        esp3d_log_w("Got error %d %s", err, esp_err_to_name(err));
                        if (err == ESP_ERR_NVS_NOT_FOUND) {
                            esp3d_log_w("Not found value for %s, use default %s", key.c_str(), query->defaultval);
                            strcpy(out_str,query->defaultval);
                            if (haserror) {
                                *haserror =false;
                            }
                            return out_str;
                        }
                    }
                    esp3d_log_e("Read %s failed: %s", key.c_str(), esp_err_to_name(err) );
                }
            } else {
                if (!out_str) {
                    esp3d_log_e("Error no output buffer");
                } else {
                    esp3d_log_e("Error size are not correct got %d but should be %d", len, query->size);
                }
            }
        } else {
            esp3d_log_e("Error setting is not a string");
        }
    } else {
        esp3d_log_e("Cannot find %d entry", index);
    }
    if (haserror) {
        *haserror =true;
    }
    esp3d_log_e("Error reading setting value");
    return "";
}

bool Esp3DSettings::writeByte (esp3d_setting_index_t index, const uint8_t value)
{
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_byte) {

            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                esp3d_log_e("Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                if (handle->set_item(key.c_str(), value) == ESP_OK) {
                    return (handle->commit()== ESP_OK);
                }
            }
        }
    }
    return false;
}

bool Esp3DSettings::writeUint32 (esp3d_setting_index_t index, const uint32_t value)
{
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_integer || query->type== esp3d_ip) {

            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                esp3d_log_e("Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                if (handle->set_item(key.c_str(), value) == ESP_OK) {
                    return (handle->commit()== ESP_OK);
                }
            }
        }
    }
    return false;
}

bool Esp3DSettings::writeIPString (esp3d_setting_index_t index, const char * byte_buffer)
{
    uint32_t ipInt = StringtoIPUInt32(byte_buffer);
    std::string ipStr = IPUInt32toString(ipInt);
    esp3d_log("write setting %d : %s to %ld to %s",index,byte_buffer,ipInt,ipStr.c_str());

    return writeUint32 (index,StringtoIPUInt32(byte_buffer));
}

bool Esp3DSettings::writeString (esp3d_setting_index_t index, const char * byte_buffer )
{
    esp3d_log("write setting %d : (%d bytes) %s ",index,strlen(byte_buffer),byte_buffer);
    const esp3d_setting_desc_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_string && strlen(byte_buffer)<= query->size) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                esp3d_log_e("Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                if (handle->set_string(key.c_str(), byte_buffer) == ESP_OK) {
                    esp3d_log("Write success");
                    return (handle->commit()== ESP_OK);
                } else {
                    esp3d_log_e("Write failed");
                }
            }
        } else {
            esp3d_log_e("Incorrect valued");
        }
    } else {
        esp3d_log_e("Unknow setting");
    }
    return false;
}

const char *Esp3DSettings::IPUInt32toString(uint32_t ip_int)
{
    ip_addr_t tmpip ;
    tmpip.type = IPADDR_TYPE_V4;
    //convert uint32_t to ip_addr_t
    ip_addr_set_ip4_u32_val(tmpip, ip_int);
    //convert ip_addr_t to_string
    return ip4addr_ntoa(&tmpip.u_addr.ip4);
}

uint32_t Esp3DSettings::StringtoIPUInt32(const char *s)
{
    ip_addr_t tmpip ;
    esp3d_log( "convert %s", s);
    tmpip.type = IPADDR_TYPE_V4;
    //convert string to ip_addr_t
    ip4addr_aton(s, &tmpip.u_addr.ip4);
    //convert ip_addr_t to uint32_t
    return ip4_addr_get_u32(&tmpip.u_addr.ip4);
}


const esp3d_setting_desc_t * Esp3DSettings::getSettingPtr(const esp3d_setting_index_t index)
{
    for (uint16_t i = 0; i < sizeof(Esp3DSettingsData); i++) {
        if (Esp3DSettingsData[i].index == index ) {
            return &Esp3DSettingsData[i];
        }
    }
    return nullptr;
}