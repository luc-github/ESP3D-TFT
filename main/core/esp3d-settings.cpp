
/*
  esp3d-settings.cpp -  settings esp3d functions class

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
#include "esp3d-settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_system.h"
#include <cstring>
#include <string>
#include "esp_log.h"


#define STORAGE_NAME "ESP3D_TFT"
#define LOG_TAG "ESP3D_SETTINGS"
#define SETTING_VERSION "ESP3D_TFT-V3.0"
#define SIZE_OF_SETTING_VERSION 25

Esp3DSettings esp3dTFTsettings;

//value of settings
const Esp3DSetting_t Esp3DSettingsData [] = {

    {esp3d_version, esp3d_string, SIZE_OF_SETTING_VERSION,"Invalid data"}
};

bool Esp3DSettings::isValidSettingsNvs()
{
    char result[SIZE_OF_SETTING_VERSION+1]  = {0};
    if (esp3dTFTsettings.readString(esp3d_version, result,SIZE_OF_SETTING_VERSION+1)) {
        if (strcmp(SETTING_VERSION, result)!=0) {
            ESP_LOGI(LOG_TAG, "Expected %s but got %s", SETTING_VERSION, result);
            return false;
        } else {
            return true;
        }
    } else {
        ESP_LOGI(LOG_TAG, "Cannot read setting version");
        return false;
    }
}

bool Esp3DSettings::reset()
{
    ESP_LOGI(LOG_TAG, "Resetting NVS");
    bool result = true;
    esp_err_t err;
    //clear all settings first
    nvs_handle_t handle_erase;
    err = nvs_open(STORAGE_NAME, NVS_READWRITE, &handle_erase);
    if (err != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Failing accessing NVS");
        return false;
    }
    if(nvs_erase_all(handle_erase)!= ESP_OK) {
        ESP_LOGE(LOG_TAG, "Failing erasing NVS");
        return false;
    }
    nvs_close(handle_erase);

    //Init each setting with default value
    //this parsing method is to workaround parsing array of enums and usage of sizeof(<array of enums>) generate
    //warning: iteration 1 invokes undefined behavior [-Waggressive-loop-optimizations]
    //may be use a vector of enum would solve also
    //fortunaly this is to reset all settings so it is ok for current situation and it avoid to create an array of enums
    for (size_t i = esp3d_version; i !=last_one; i++) {
        esp3d_setting_index_t setting  =(esp3d_setting_index_t)i ;
        const Esp3DSetting_t * query = getSettingPtr(setting);
        if (query) {
            switch(query->type) {
            case  esp3d_byte:
                if (!Esp3DSettings::writeByte (setting, (uint8_t)std::stoul(std::string(query->defaultval), NULL,0) )) {

                    result = false;
                }
                break;
            case esp3d_ip:
            case esp3d_integer:
                if (!Esp3DSettings::writeUint32 (setting, (uint32_t)std::stoul(std::string(query->defaultval), NULL,0) )) {
                    result = false;
                }
                break;
            case esp3d_string:
                if (setting==esp3d_version) {
                    if (!Esp3DSettings::writeString (setting, SETTING_VERSION)) {
                        result = false;
                    }
                } else {
                    if (!Esp3DSettings::writeString (setting, query->defaultval)) {
                        result = false;
                    }
                }

                break;
            default:
                result =false; //type is not handle
            }
        } else {
            result =false; //setting invalid
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
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_byte) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failling accessing NVS");
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
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_integer || query->type== esp3d_ip) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failling accessing NVS");
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
                    value = (uint32_t)std::stoul(std::string(query->defaultval), NULL,0);
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

const char* Esp3DSettings::readString(esp3d_setting_index_t index, char* out_str, size_t len, bool * haserror)
{
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query) {
        if (query->type== esp3d_string) {
            esp_err_t err;
            if (out_str && len >= (query->size +1)) {
                std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READONLY, &err);
                if (err != ESP_OK) {
                    ESP_LOGE(LOG_TAG, "Failling accessing NVS");
                } else {
                    std::string key = "p_"+std::to_string((uint)query->index);
                    err = handle->get_string(key.c_str(), out_str, query->size);
                    if (err== ESP_OK) {
                        if (haserror) {
                            *haserror =false;
                        }
                        return out_str;
                    }
                    if (err == ESP_ERR_NVS_NOT_FOUND) {
                        strcpy(out_str,query->defaultval);
                        if (haserror) {
                            *haserror =false;
                        }
                        return out_str;
                    }
                    ESP_LOGI(LOG_TAG, "Read %s failed: %s", key.c_str(), esp_err_to_name(err) );
                }
            }
        }
    }
    if (haserror) {
        *haserror =true;
    }
    return NULL;
}

bool Esp3DSettings::writeByte (esp3d_setting_index_t index, const uint8_t value)
{
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_byte) {

            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failling accessing NVS");
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
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_integer || query->type== esp3d_ip) {

            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failling accessing NVS");
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
bool Esp3DSettings::writeString (esp3d_setting_index_t index, const char * byte_buffer )
{
    const Esp3DSetting_t * query = getSettingPtr(index);
    if (query ) {
        if (query->type== esp3d_string && strlen(byte_buffer)<= query->size) {
            esp_err_t err;
            std::shared_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(STORAGE_NAME, NVS_READWRITE, &err);
            if (err != ESP_OK) {
                ESP_LOGE(LOG_TAG, "Failling accessing NVS");
            } else {
                std::string key = "p_"+std::to_string((uint)query->index);
                if (handle->set_string(key.c_str(), byte_buffer) == ESP_OK) {
                    return (handle->commit()== ESP_OK);
                }
            }
        }
    }
    return false;
}


const Esp3DSetting_t * Esp3DSettings::getSettingPtr(esp3d_setting_index_t index)
{
    for (uint16_t i = 0; i < sizeof(Esp3DSettingsData); i++) {
        if (Esp3DSettingsData[i].index == index ) {
            return &Esp3DSettingsData[i];
        }
    }
    return nullptr;
}