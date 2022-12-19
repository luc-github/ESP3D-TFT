/*
  esp3d_notifications
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

#include "esp3d_notifications_service.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_crt_bundle.h"
#include "network/esp3d_network.h"


#define STEP_EMAIL 0
#define STEP_ADDRESS 1
#define STEP_PORT 2

//Email#serveraddress:port
bool Esp3DNotificationsService::getEmailInformationsFromSettings()
{
    char buffer[SIZE_OF_SETTING_NOFIFICATION_TS+1];
    esp3dTFTsettings.readString(esp3d_notification_token_setting, buffer, SIZE_OF_SETTING_NOFIFICATION_TS);
    //0 = email, 1 =server address, 2 = port
    _serveraddress="https://";
    uint8_t step = STEP_EMAIL;
    for(uint i = 0; i < strlen(buffer); i++) {
        if(buffer[i] ==':' || buffer[i] == '#') {
            step++;
        } else {
            switch (step) {
            case STEP_EMAIL:
                _settings += buffer[i];
                break;
            case STEP_ADDRESS:
                _serveraddress += buffer[i];
                break;
            case STEP_PORT:
                _port += buffer[i];
                break;
            default :
                return false;
            }
        }
    }
    if (step!=2) {
        esp3d_log_e("Missing some Parameters");
        return false;
    }
    esp3d_log("Server: %s, port: %s, email: %s", _serveraddress.c_str(),_port.c_str(), _settings.c_str());
    return true;
}

bool Esp3DNotificationsService::sendEmailMSG(const char * title, const char * message)
{
    if (_token1.length() == 0 || _token2.length()==0  || _settings.length()==0 || _port.length()==0 || _serveraddress.length()) {
        esp3d_log_e("Some token is missing");
        return false;
    }
    //TODO
    return true;
}
