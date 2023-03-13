/*
  esp3d_tft project

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

#pragma once
/*Notification Title used according notification type*/
#define ESP3D_NOTIFICATION_TITLE "Hi from ESP3D"
/* Notification message when online
 * The message that will be sent when the ESP is online
 */
#define ESP3D_NOTIFICATION_ONLINE "Hi, %ESP_NAME% is now online at %ESP_IP%"

/* Sender email to be used for sending notification email
 * if not defined it use the destination address provided in TS
 */
// #define ESP3D@mysmtp.com