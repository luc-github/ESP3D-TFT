/*
  esp3d_network
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

#include "esp3d_update_service.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include <cstdlib>
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "filesystem/esp3d_sd.h"
#include "network/esp3d_network.h"
#include "esp3d_config_file.h"

#define CONFIG_FILE "/esp3dcnf.ini"
#define CONFIG_FILE_OK "/esp3dcnf.ok"
#define FW_FILE "/esp3dfw.bin"
#define FW_FILE_OK "/esp3dfw.ok"
//#define FS_FILE "/esp3dfs.bin"
#define CHUNK_BUFFER_SIZE 1024

Esp3DUpdateService esp3dUpdateService;

const char * protectedkeys[] = {"NOTIF_TOKEN1","NOTIF_TOKEN2","AP_Password","STA_Password","ADMIN_PASSWORD","USER_PASSWORD"} ;






//Network
//String values
const char * NetstringKeysVal[] = {"hostname",
                                   "STA_SSID",
                                   "STA_Password",
                                   "AP_SSID",
                                   "AP_Password"
                                  } ;

const esp3d_setting_index_t NetstringKeysPos[] = {esp3d_hostname,
                                                  esp3d_sta_ssid,
                                                  esp3d_sta_password,
                                                  esp3d_ap_ssid,
                                                  esp3d_ap_password
                                                 };

//IP values
const char * IPKeysVal[] = {"STA_IP",
                            "STA_GW",
                            "STA_MSK",
                            "STA_DNS",
                            "AP_IP"
                           };

const esp3d_setting_index_t IPKeysPos[] = {esp3d_sta_ip_static,
                                           esp3d_sta_gw_static,
                                           esp3d_sta_mask_static,
                                           esp3d_sta_dns_static,
                                           esp3d_ap_ip_static
                                          };
//Bytes values
const char * NetbyteKeysVal[] = {
    "AP_channel"
};

const esp3d_setting_index_t NetbyteKeysPos[] = {
    esp3d_ap_channel
};

//Services
//String values
const char * ServstringKeysVal[] = {
//  "Time_server1",
//  "Time_server2",
//   "Time_server3",
//  "ADMIN_PASSWORD",
//  "USER_PASSWORD",
//   "NOTIF_TOKEN1",
//   "NOTIF_TOKEN2",
//    "NOTIF_TOKEN_Settings"
};

const esp3d_setting_index_t ServstringKeysPos[] = {
//   ESP_TIME_SERVER1,
//   ESP_TIME_SERVER2,
//   ESP_TIME_SERVER3,
//   ESP_ADMIN_PWD,
//   ESP_USER_PWD,
//   ESP_NOTIFICATION_TOKEN1,
//   ESP_NOTIFICATION_TOKEN2,
//   ESP_NOTIFICATION_SETTINGS
} ;

//Integer values
const char * ServintKeysVal[] = {
    "HTTP_Port"
//    "TELNET_Port",
//    "SENSOR_INTERVAL",
//    "WebSocket_Port",
//    "WebDav_Port",
//    "FTP_Control_Port",
//    "FTP_Active_Port ",
//    "FTP_Passive_Port"
};

const esp3d_setting_index_t ServintKeysPos[] = {
    esp3d_http_port
//    ESP_TELNET_PORT,
//    ESP_SENSOR_INTERVAL,
//    ESP_WEBSOCKET_PORT,
//    ESP_WEBDAV_PORT,
//    ESP_FTP_CTRL_PORT,
//    ESP_FTP_DATA_ACTIVE_PORT,
//    ESP_FTP_DATA_PASSIVE_PORT
};

//Boolean values
const char * ServboolKeysVal[] = {
    "HTTP_active",
    //"TELNET_active",
    //"WebSocket_active",
    //"WebDav_active",
    //"Time_DST",
    "CHECK_FOR_UPDATE",
    //"Active_buzzer",
    //"Active_Internet_time",
    "Radio_enabled"
} ;

const esp3d_setting_index_t ServboolKeysPos[] = {
    esp3d_http_on,
    //ESP_TELNET_ON,
    //ESP_WEBSOCKET_ON,
    //ESP_WEBDAV_ON,
    //ESP_TIME_IS_DST,
    esp3d_check_update_on_sd,
    //ESP_BUZZER,
    //ESP_INTERNET_TIME,
    esp3d_radio_boot_mode
} ;

//Byte values
const char * ServbyteKeysVal[] = {
    //"Time_zone",
    //"Sesion_timeout",
    "SD_SPEED"
    //"Time_DST"
} ;

const esp3d_setting_index_t ServbyteKeysPos[] = {
    //ESP_TIMEZONE,
    //ESP_SESSION_TIMEOUT,
    esp3d_spi_divider
    //ESP_TIME_DST
} ;

//System
//Integer values
const char * SysintKeysVal[] = {"Baud_rate"
                                //"Boot_delay"
                               };

const esp3d_setting_index_t SysintKeysPos[] = {esp3d_baud_rate
                                               //ESP_BOOT_DELAY
                                              };
//Boolean values
const char * SysboolKeysVal[] = {
    //"Active_Serial ",
    //"Active_WebSocket",
    //"Active_Telnet",
    //"Active_BT",
    //"Boot_verbose",
    //"Secure_serial"
};

const esp3d_setting_index_t SysboolKeysPos[] = {
    // ESP_SERIAL_FLAG,
    // ESP_WEBSOCKET_FLAG,
    // ESP_TELNET_FLAG,
    // ESP_BT_FLAG,
    // ESP_VERBOSE_BOOT,
    //ESP_SECURE_SERIAL
};


Esp3DUpdateService::Esp3DUpdateService()
{
}

Esp3DUpdateService::~Esp3DUpdateService() {}

bool Esp3DUpdateService::canUpdate()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *update_partition =  esp_ota_get_next_update_partition(NULL);
    if (!running) {
        esp3d_log_e ("Cannot get running partition");
        return false;
    }
    esp_app_desc_t running_app_info;
    esp3d_log("Running partition subtype %d at offset 0x%lx",
              running->subtype, running->address);
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        esp3d_log("Running firmware version: %s", running_app_info.version);
    }
    if (!update_partition) {
        esp3d_log_e ("Cannot get update partition");
        return false;
    }
    esp3d_log("Update partition subtype %d at offset 0x%lx",
              update_partition->subtype, update_partition->address);
    return true;
}

size_t Esp3DUpdateService::maxUpdateSize()
{
    size_t max_size = 0;
    const esp_partition_t *update_partition =  esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        esp3d_log_e ("Cannot get update partition");
    } else {
        max_size = update_partition->size;
    }
    return max_size;
}

bool Esp3DUpdateService::begin()
{
    esp3d_log("Starting Update Service");
    bool restart =false;
    esp3d_state_t setting_check_update = (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_check_update_on_sd);
    if (setting_check_update==esp3d_state_off || !canUpdate()) {
        esp3d_log("Update Service disabled");
        return true;
    }
    if (sd.accessFS()) {
        if (sd.exists(FW_FILE)) {
            restart= updateFW();
            if (restart) {
                std::string filename =FW_FILE_OK;
                uint8_t n = 1;
                //if FW_FILE_OK already exists,  rename it to FW_FILE_OKXX
                if (sd.exists(filename.c_str())) {
                    //find proper name
                    while (sd.exists(filename.c_str())) {
                        filename = FW_FILE_OK + std::to_string(n++);
                    }
                    //rename FW_FILE_OK to FW_FILE_OKXX
                    if (!sd.rename(FW_FILE_OK,filename.c_str())) {
                        esp3d_log_e("Failed to rename %s",FW_FILE_OK);
                        //to avoid dead loop
                        restart = false;
                    }
                }
                //rename FW_FILE to FW_FILE_OK
                if (!sd.rename(FW_FILE,FW_FILE_OK)) {
                    esp3d_log_e("Failed to rename %s",FW_FILE);
                    //to avoid dead loop
                    restart = false;
                }
            }
        } else {
            esp3d_log("No Fw update on SD");
            if (sd.exists(CONFIG_FILE)) {
                restart= updateConfig();

            }
        }
        sd.releaseFS();
    } else {
        esp3d_log("SD unavailable for update");
    }
    if (restart) {
        esp3d_log("Restarting  firmware");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    return true;
}

bool Esp3DUpdateService::updateConfig()
{
    bool res = false;
    Esp3DConfigFile updateConfig(CONFIG_FILE, esp3dUpdateService.processingFileFunction,CONFIG_FILE_OK, protectedkeys);
    if (updateConfig.processFile()) {
        esp3d_log("Processing ini file done");
        if(updateConfig.revokeFile()) {
            esp3d_log("Revoking ini file done");
            res = true;
        } else {
            esp3d_log_e("Revoking ini file failed");
        }
    } else {
        esp3d_log_e("Processing ini file failed");
    }
    return res;
}

bool Esp3DUpdateService::updateFW()
{
    bool isSuccess = true;
    char chunk[CHUNK_BUFFER_SIZE];
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    esp3d_log("Updating firmware");
    struct stat entry_stat;
    size_t totalSize = 0;
    if (sd.stat(FW_FILE, &entry_stat) == -1) {
        esp3d_log_e("Failed to stat : %s", FW_FILE);
        return false;
    }
    FILE* fwFd = sd.open(FW_FILE, "r");
    if (!fwFd) {
        esp3d_log_e("Failed to open on sd : %s", FW_FILE);
        return false;
    }
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        esp3d_log_e("Error accessing flash filesystem");
        isSuccess = false;
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
        if (err != ESP_OK) {
            esp3d_log_e("esp_ota_begin failed (%s)", esp_err_to_name(err));
            isSuccess = false;
        }
    }
    if (isSuccess) {
        size_t chunksize ;
        uint8_t progress =0;
        do {
            chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, fwFd);
            totalSize+=chunksize;
            if (esp_ota_write( update_handle, (const void *)chunk, chunksize)!=ESP_OK) {
                esp3d_log_e("Error cannot writing data on update partition");
                isSuccess = false;
            }
#if ESP3D_TFT_LOG
            if ( progress != 100*totalSize/entry_stat.st_size) {
                progress = 100*totalSize/entry_stat.st_size;
                esp3d_log("Update %d %% %d / %ld", progress, totalSize,entry_stat.st_size );
            }
#endif
        } while (chunksize != 0 && isSuccess);
    }
    sd.close(fwFd);
    if (isSuccess) {
        //check size
        if (totalSize!= entry_stat.st_size) {
            esp3d_log_e("Failed to read firmware full data");
            isSuccess = false;
        }
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_end(update_handle);
        if (err!=ESP_OK) {
            esp3d_log_e("Error cannot end update(%s)", esp_err_to_name(err));
            isSuccess = false;
        }
        update_handle = 0;
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_set_boot_partition(update_partition);
        if (err!=ESP_OK) {
            esp3d_log_e( "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
            isSuccess = false;
        }
    }
    if (update_handle && !isSuccess) {
        esp_ota_abort(update_handle);
        update_handle = 0;
    }
    return isSuccess;
}

void Esp3DUpdateService::handle() {}

void Esp3DUpdateService::end()
{
    esp3d_log("Stop Update Service");
}


bool Esp3DUpdateService::processString(const char** keysval, const esp3d_setting_index_t * keypos, const size_t size, const char * key, const char * value, char & T, esp3d_setting_index_t & P )
{

    for(uint i=0; i< size ; i++) {
        if (strcasecmp(keysval[i],key)==0) {
            //if it is a previouly saved scrambled password ignore it
            if (strcasecmp(value,"********")!=0) {
                T='S';
                P=keypos[i];
                esp3d_log("It is a string key");
                return true;
            }
        }
    }
    esp3d_log("Not a string key");
    return false;
}

bool Esp3DUpdateService::processInt(const char** keysval, const esp3d_setting_index_t * keypos, const size_t size, const char * key, const char * value, char & T, esp3d_setting_index_t & P,  uint32_t & v)
{
    for(uint i=0; i< size ; i++) {
        if (strcasecmp(keysval[i],key)==0) {
            T='I';
            P=keypos[i];
            v=atoi(value);
            esp3d_log("It is an integer key");
            return true;
        }
    }
    esp3d_log("Not an integer key");
    return false;
}

bool Esp3DUpdateService::processBool(const char** keysval, const esp3d_setting_index_t * keypos, const size_t size, const char * key, const char * value, char & T, esp3d_setting_index_t & P,  uint8_t & b)
{
    for(uint i=0; i< size ; i++) {
        if (strcasecmp(keysval[i],key)==0) {
            T='B';
            P=keypos[i];
            if ((strcasecmp("yes",value)==0)||(strcasecmp("on", value)==0)||(strcasecmp("true", value)==0)||(strcasecmp("1", value)==0) ) {
                b = 1;
            } else if ((strcasecmp("no", value)==0)||(strcasecmp("off", value)==0)||(strcasecmp("false", value)==0)||(strcasecmp("0", value)==0) ) {
                b = 0;
            } else {
                esp3d_log("Not a valid boolean key");
                return false;
            }
            esp3d_log("Not a boolean key");
            return true;
        }
    }
    esp3d_log("Not a boolean key");
    return false;
}

bool Esp3DUpdateService::processingFileFunction (const char * section, const char * key, const char * value)
{
    esp3d_log("Processing Section: %s, Key: %s, Value: %s", section, key, value);
    bool res = true;
    char T = '\0';
    esp3d_setting_index_t P = last_esp3d_setting_index_t;
    uint32_t v = 0;
    uint8_t b = 0;
    bool done=false;
    //network / services / system sections
    if (strcasecmp("network",section)==0) {
        if (!done) {
            done = processString(NetstringKeysVal,NetstringKeysPos,sizeof(NetstringKeysVal)/sizeof(char*),  key, value, T, P );
        }
        if (!done) {
            done = processString(IPKeysVal,IPKeysPos,sizeof(IPKeysVal)/sizeof(char*),  key, value, T, P );
            if(done) {
                T='A';
            }
        }
        if (!done) {
            done = processInt(NetbyteKeysVal,NetbyteKeysPos,sizeof(NetbyteKeysVal)/sizeof(char*),  key, value, T, P, v);
            if(done) {
                T='B';
                b=v;
            }
        }
        //Radio mode BT, STA, AP, OFF
        if (!done) {
            if (strcasecmp("radio_mode",key)==0) {
                T='B';
                P=esp3d_radio_mode;
                done = true;
                if (strcasecmp("BT",value)==0) {
                    b=esp3d_bluetooth_serial;
                } else if (strcasecmp("STA",value)==0) {
                    b=esp3d_wifi_sta;
                } else if (strcasecmp("AP",value)==0) {
                    b=esp3d_wifi_ap;
                } else if (strcasecmp("SETUP",value)==0) {
                    b=esp3d_wifi_ap_config;
                } else if (strcasecmp("OFF",value)==0) {
                    b=esp3d_radio_off;
                } else {
                    P= last_esp3d_setting_index_t;    //invalid value
                }
            }
        }
        //STA fallback mode BT, WIFI-AP, OFF
        if (!done) {
            if (strcasecmp("sta_fallback",key)==0) {
                T='B';
                P = esp3d_fallback_mode;
                done = true;
                if (strcasecmp("BT",value)==0) {
                    b=esp3d_bluetooth_serial;
                } else if (strcasecmp("WIFI-SETUP",value)==0) {
                    b=esp3d_wifi_ap_config;
                } else if (strcasecmp("OFF",value)==0) {
                    b=esp3d_radio_off;
                } else {
                    P= last_esp3d_setting_index_t;    //invalid value
                }
            }
        }

        //STA IP Mode DHCP / STATIC
        if (!done) {
            if (strcasecmp("STA_IP_mode",key)==0) {
                T='B';
                P=esp3d_sta_ip_mode;
                done = true;
                if (strcasecmp("DHCP",value)==0) {
                    b=esp3d_ip_mode_dhcp;
                } else if (strcasecmp("STATIC",key)==0) {
                    b=esp3d_ip_mode_static;
                } else {
                    P= last_esp3d_setting_index_t;    //invalid value
                }
            }
        }
    } else if (strcasecmp("services",section)==0) {
        if (!done) {
            done = processString(ServstringKeysVal,ServstringKeysPos,sizeof(ServstringKeysVal)/sizeof(char*),  key, value, T, P );
        }
        if (!done) {
            done = processInt(ServintKeysVal,ServintKeysPos,sizeof(ServintKeysVal)/sizeof(char*),  key, value, T, P, v);
        }
        if (!done) {
            done = processBool(ServboolKeysVal,ServboolKeysPos,sizeof(ServboolKeysVal)/sizeof(char*),  key, value, T, P, b);
        }
        if (!done) {
            done = processInt(ServbyteKeysVal,ServbyteKeysPos,sizeof(ServbyteKeysVal)/sizeof(char*),  key, value, T, P, v);
            if(done) {
                T='B';
                b=v;
            }
        }
        //Notification type None / PushOver / Line / Email / Telegram / IFTTT
        /*  if (!done) {
              if (strcasecmp("NOTIF_TYPE",key)==0) {
                  T='B';
                  P=ESP_NOTIFICATION_TYPE;
                  done = true;
                  if (strcasecmp("None",value)==0) {
                      b=ESP_NO_NOTIFICATION;
                  } else if (strcasecmp("PushOver",value)==0) {
                      b=ESP_PUSHOVER_NOTIFICATION;
                  } else if (strcasecmp("Line",value)==0) {
                      b=ESP_LINE_NOTIFICATION;
                  } else if (strcasecmp("Email",value)==0) {
                      b=ESP_EMAIL_NOTIFICATION;
                  } else if (strcasecmp("Telegram",value)==0) {
                      b=ESP_TELEGRAM_NOTIFICATION;
                  } else if (strcasecmp("IFTTT",value)==0) {
                      b=ESP_IFTTT_NOTIFICATION;
                  } else {
                      P=-1;    //invalide value
                  }
              }
          }*/
    } else if (strcasecmp("system",section)==0) {
        if (!done) {
            done = processInt(SysintKeysVal,SysintKeysPos,sizeof(SysintKeysVal)/sizeof(char*),  key, value, T, P, v);
        }
        if (!done) {
            done = processBool(SysboolKeysVal,SysboolKeysPos,sizeof(SysboolKeysVal)/sizeof(char*),  key, value, T, P, b);
        }
        //Target Firmware None / Marlin / Repetier / MarlinKimbra / Smoothieware / GRBL
        if (!done) {
            if (strcasecmp("TargetFW",key)==0) {
                T='B';
                P=esp3d_target_firmware;
                done = true;
                if (strcasecmp("None",value)==0) {
                    b=esp3d_unknown;
                } else if (strcasecmp("MARLIN",value)==0) {
                    b=esp3d_marlin;
                } else if (strcasecmp("GRBLHAL",value)==0) {
                    b=esp3d_grblhal;
                } else if (strcasecmp("GRBL",value)==0) {
                    b=esp3d_grbl;
                } else if (strcasecmp("REPETIER",value)==0) {
                    b=esp3d_repetier;
                } else if (strcasecmp("SMOOTHIEWARE",value)==0) {
                    b=esp3d_smoothieware;
                } else if (strcasecmp("HP_GL",value)==0) {
                    b=esp3d_hp_gl;
                } else {
                    P=last_esp3d_setting_index_t;    //invalid value
                }
            }
        }
    }

    //now we save - handle saving status
    //if setting is not recognized it is not a problem
    //but if save is fail - that is a problem - so report it
    if(P!=last_esp3d_setting_index_t) {
        switch(T) {
        case 'S':
            if (esp3dTFTsettings.isValidStringSetting(value,P)) {
                res = esp3dTFTsettings.writeString (P, value);
            } else {
                esp3d_log_e("Value \"%s\" is invalid", value);
            }
            break;
        case 'B':
            if (esp3dTFTsettings.isValidByteSetting(b, P)) {
                res = esp3dTFTsettings.writeByte (P, b);
            } else {
                esp3d_log_e("Value \"%d\" is invalid", b);
            }
            break;
        case 'I':
            if (esp3dTFTsettings.isValidIntegerSetting(v, P)) {
                res = esp3dTFTsettings.writeUint32 (P, v);
            }  else {
                esp3d_log_e("Value \"%ld\" is invalid", v);
            }
            break;
        case 'A':
            if (esp3dTFTsettings.isValidIPStringSetting(value,P)) {
                res = esp3dTFTsettings.writeIPString (P, value);
            } else {
                esp3d_log_e("Value \"%s\" is invalid", value);
            }
            break;
        default:
            esp3d_log_e("Unknown flag");
        }
    } else {
        esp3d_log_e("Unknown value");
    }
    return res;
}