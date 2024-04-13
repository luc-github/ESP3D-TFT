# #######################################
# Select the targeted hardware
# #######################################

# With TFT:
OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" OFF)
OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF)
OPTION(ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" OFF)
OPTION(ESP32S3_BZM_TFT35_GT911 "TFT TARGET is ESP32S3 Panlee BZM 3.5inches" ON)
OPTION(ESP32S3_8048S070C "TFT TARGET is ESP32S3_8048S070C - 7.0in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_8048S050C "TFT TARGET is ESP32S3_8048S050C - 5.0in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_8048S043C "TFT TARGET is ESP32S3_8048S043C - 4.3in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_4827S043C "TFT TARGET is ESP32S3_4827S043C - 4.3in. 480x272 (Capacitive)" OFF)
OPTION(ESP32_3248S035C "TFT TARGET is ESP32_3248S035C - 3.5in. 480x320 (Capacitive)" OFF)
OPTION(ESP32_3248S035R "TFT TARGET is ESP32_3248S035R - 3.5in. 480x320 (Resistive)" OFF)
OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R - 2.8in. 320x240 (Resistive)" OFF)

# Without TFT:
OPTION(ESP32S3_FREENOVE_1_1 "HARDWARE TARGET is ESP32S3 Freenove v1.1" OFF)
OPTION(ESP32S3_SEEED_STUDIO_XIAO  "HARDWARE TARGET is ESP32S3 Seeed Studio Xiao Sense" OFF)
OPTION(ESP32S3_CUSTOM "HARDWARE TARGET is ESP32S3 custom board" OFF)
OPTION(ESP32_CUSTOM "HARDWARE TARGET is ESP32 custom board" OFF)


# #######################################
# Optionally: Select hardware mods applied
# #######################################
#only for ESP32S3_8048S070C OR ESP32S3_8048S050C OR ESP32S3_8048S043C OR ESP32S3_4827S043C OR ESP32_3248S035C
OPTION(HARDWARE_MOD_GT911_INT "Hardware Mod: GT911 INT pin" OFF)

#Only for ESP32_3248S035C OR ESP32_3248S035R OR ESP32_2432S028R
# 4MB Flash, no PSRAM by default
OPTION(HARDWARE_MOD_8MB_FLASH "Hardware Mod: 8MB Flash Upgrade" OFF)
OPTION(HARDWARE_MOD_16MB_FLASH "Hardware Mod: 16MB Flash Upgrade" OFF)
OPTION(HARDWARE_MOD_EXT_PSRAM "Hardware Mod: External PSRAM" OFF)



# #######################################
# Select the targeted firmware
# #######################################
OPTION(TARGET_FW_MARLIN "Marlin firmware" ON)
OPTION(TARGET_FW_REPETIER "Repetier firmware" OFF)
OPTION(TARGET_FW_SMOOTHIEWARE "Smoothieware firmware" OFF)
OPTION(TARGET_FW_GRBL "GRBL firmware" OFF)


# #######################################
# Select the Features
# #######################################
OPTION(ESP3D_AUTHENTICATION "Authentication on all clients" OFF)
OPTION(DISABLE_SERIAL_AUTHENTICATION "Disable Serial Authentication" ON)
OPTION(TIME_SERVICE  "Time service" ON)
OPTION(SSDP_SERVICE "SSDP service" ON)
OPTION(MDNS_SERVICE "MDNS service" ON)
OPTION(WIFI_SERVICE "WiFi service" ON)
OPTION(BT_SERVICE "Bluetooth service" ON)
OPTION(WEB_SERVICES "Web Services http/websocket/webdav/camera" ON)
OPTION(CAMERA_SERVICE "Camera service" ON)
OPTION(WEBDAV_SERVICES "WebDav Services" ON)
OPTION(TELNET_SERVICE "Telnet service" ON)
OPTION(WS_SERVICE "WebSocket data service" ON)
OPTION(TFT_UI_SERVICE "TFT UI service" ON)
OPTION(SD_CARD_SERVICE "SD card service" ON)
OPTION(NOTIFICATIONS_SERVICE "Notifications service" ON)
OPTION(UPDATE_SERVICE "Update service" ON)
OPTION(USB_SERIAL_SERVICE "Use USB Serial if Available" ON)
OPTION(USE_FAT_INSTEAD_OF_LITTLEFS "Use FAT instead of LittleFS" OFF)

# #######################################
# Do not change anything below this line
# #######################################
cmake_minimum_required(VERSION 3.12.4)
set(CMAKE_CXX_STANDARD 20)

include (cmake/targets.cmake)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

include (cmake/features.cmake)

project(ESP3D-TFT
    VERSION 1.0
    DESCRIPTION "ESP3D TFT")

