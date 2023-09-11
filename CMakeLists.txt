# #######################################
# Select the targeted hardware
# #######################################


OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" ON)
OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF)
OPTION(ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" OFF)
OPTION(ESP32S3_BZM_TFT35_GT911 "TFT TARGET is ESP32S3 Panlee BZM 3.5inches" OFF)
OPTION(ESP32S3_8048S070C "TFT TARGET is ESP32S3_8048S070C - 7.0in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_8048S050C "TFT TARGET is ESP32S3_8048S050C - 5.0in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_8048S043C "TFT TARGET is ESP32S3_8048S043C - 4.3in. 800x480 (Capacitive)" OFF)
OPTION(ESP32S3_4827S043C "TFT TARGET is ESP32S3_4827S043C - 4.3in. 480x272 (Capacitive)" OFF)

OPTION(ESP32S3_FREENOVE_1_1 "HARDWARE TARGET is ESP32S3 Freenove v1.1" OFF)
OPTION(ESP32S3_CUSTOM "HARDWARE TARGET is ESP32S3 custom board" OFF)
OPTION(ESP32_CUSTOM "HARDWARE TARGET is ESP32 custom board" OFF)

# WARNING:
#   Limited help or support available for these targets.
#   PSRAM hardware mod is REQUIRED for full functionality and performance.
OPTION(ESP32_3248S035C "TFT TARGET is ESP32_3248S035C - 3.5in. 480x320 (Capacitive)" OFF)
OPTION(ESP32_3248S035R "TFT TARGET is ESP32_3248S035R - 3.5in. 480x320 (Resistive)" OFF)
OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R - 2.8in. 320x240 (Resistive)" OFF)


# #######################################
# Optionally: Select hardware mods applied
# #######################################
if(ESP32S3_8048S070C OR ESP32S3_8048S050C OR ESP32S3_8048S043C OR ESP32S3_4827S043C OR ESP32_3248S035C)
    OPTION(HARDWARE_MOD_GT911_INT "Hardware Mod: GT911 INT pin" OFF)
endif()
if(ESP32_3248S035C OR ESP32_3248S035R OR ESP32_2432S028R)
    # 4MB Flash, no PSRAM by default
    OPTION(HARDWARE_MOD_8MB_FLASH "Hardware Mod: 8MB Flash Upgrade" OFF)
    OPTION(HARDWARE_MOD_16MB_FLASH "Hardware Mod: 16MB Flash Upgrade" OFF)
    OPTION(HARDWARE_MOD_EXT_PSRAM "Hardware Mod: External PSRAM" OFF)
endif()


# #######################################
# Select the targeted firmware
# #######################################
OPTION(TARGET_FW_MARLIN "Marlin firmware" ON)
OPTION(TARGET_FW_REPETIER "Repetier firmware" OFF)
OPTION(TARGET_FW_SMOOTHIEWARE "Smoothieware firmware" OFF)
OPTION(TARGET_FW_GRBL "GRBL firmware" OFF)


# #######################################
# Select the Options
# #######################################
OPTION(ESP3D_AUTHENTICATION "Authentication on all clients" OFF)
OPTION(DISABLE_SERIAL_AUTHENTICATION "Disable Serial Authentication" ON)
OPTION(TIMESTAMP_ON_FILES "Time on file" OFF)
OPTION(SSDP_SERVICE "SSDP service" ON)
OPTION(MDNS_SERVICE "MDNS service" ON)
OPTION(WIFI_SERVICE "WiFi service" ON)
OPTION(BT_SERVICE "Bluetooth service" ON)
OPTION(WEB_SERVICES "Web Services http/websocket" ON)
OPTION(TELNET_SERVICE "Telnet service" ON)
OPTION(WS_SERVICE "WebSocket data service" ON)
OPTION(TFT_UI_SERVICE "TFT UI service" ON)
OPTION(SD_CARD_SERVICE "SD card service" ON)
OPTION(NOTIFICATIONS_SERVICE "Notifications service" ON)
OPTION(UPDATE_SERVICE "Update service" ON)
OPTION(GCODE_HOST_SERVICE "GCode Streaming service" ON)
OPTION(USE_FAT_INSTEAD_OF_LITTLEFS "Use FAT instead of LittleFS" OFF)

# #######################################
# Do not change anything below this line
# #######################################
cmake_minimum_required(VERSION 3.12.4)
set(CMAKE_CXX_STANDARD 20)
#C:/Users/luc/Documents/GitHub/ESP3D-TFT/
set(EXTRA_COMPONENT_DIRS hardware/common/components)
if(ESP32S3_FREENOVE_1_1)
    set(TFT_TARGET "ESP32S3_FREENOVE_1_1")
    set(SDKCONFIG hardware/ESP32S3_FREENOVE_1_1/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_FREENOVE_1_1/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_FREENOVE_1_1/components/bsp")    
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
    unset(TFT_UI_SERVICE CACHE)
elseif(ESP32S3_8048S070C)
    set(TFT_TARGET "ESP32S3_8048S070C")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32-S3/with_PSRAM/16MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S070C/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S070C/components/bsp")
elseif(ESP32S3_8048S050C)
    set(TFT_TARGET "ESP32S3_8048S050C")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32-S3/with_PSRAM/16MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S050C/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S050C/components/bsp")
elseif(ESP32S3_8048S043C)
    set(TFT_TARGET "ESP32S3_8048S043C")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32-S3/with_PSRAM/16MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S043C/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S043C/components/bsp")
elseif(ESP32S3_4827S043C)
    set(TFT_TARGET "ESP32S3_4827S043C")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32-S3/with_PSRAM/16MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32S3_4827S043C/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_4827S043C/components/bsp")
elseif(ESP32_3248S035C)
    set(TFT_TARGET "ESP32_3248S035C")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32/no_PSRAM/4MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035C/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_3248S035C/components/bsp")
elseif(ESP32_3248S035R)
    set(TFT_TARGET "ESP32_3248S035R")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32/no_PSRAM/4MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035R/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_3248S035R/components/bsp")
elseif(ESP32_2432S028R)
    set(TFT_TARGET "ESP32_2432S028R")
    set(SDKCONFIG hardware/common/sdkconfig/ESP32/no_PSRAM/4MB_Flash)    
    list(APPEND EXTRA_COMPONENT_DIRS hardware/ESP32_2432S028R/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_2432S028R/components/bsp")
elseif(ESP32S3_HMI43V3)
    set(TFT_TARGET "ESP32S3_HMI43V3")    
    set(SDKCONFIG hardware/ESP32S3_HMI43V3/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_HMI43V3/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/components/bsp")
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32_ROTRICS_DEXARM35)
    set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")    
    set(SDKCONFIG hardware/ESP32_ROTRICS_DEXARM35/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_ROTRICS_DEXARM35/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components/bsp")
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    set(TFT_TARGET "ESP32S3_ZX3D50CE02S_USRC_4832")
    set(SDKCONFIG hardware/ESP32S3_ZX3D50CE02S_USRC_4832/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components/bsp")    
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32S3_BZM_TFT35_GT911)
    set(TFT_TARGET "ESP32S3_BZM_TFT35_GT911")
    set(SDKCONFIG hardware/ESP32S3_BZM_TFT35_GT911/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_BZM_TFT35_GT911/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_BZM_TFT35_GT911/components/bsp")
elseif(ESP32S3_CUSTOM)
    set(TFT_TARGET "ESP32S3_CUSTOM")
    set(SDKCONFIG hardware/ESP32S3_CUSTOM/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_CUSTOM/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_CUSTOM/components/bsp")    
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32_CUSTOM)
    set(TFT_TARGET "ESP32_CUSTOM")
    set(SDKCONFIG hardware/ESP32_CUSTOM/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_CUSTOM/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_CUSTOM/components/bsp")
else()
    message(FATAL_ERROR
        "\n"
        "No hardware target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
endif()

if(ESP32S3_8048S070C OR ESP32S3_8048S050C OR ESP32S3_8048S043C OR ESP32S3_4827S043C OR ESP32_3248S035C)
    if(HARDWARE_MOD_GT911_INT)
        add_compile_options(-DWITH_GT911_INT=1)
    endif()
endif()
if(ESP32_3248S035C OR ESP32_3248S035R OR ESP32_2432S028R)
    if(HARDWARE_MOD_EXT_PSRAM)
        add_compile_options(-DWITH_PSRAM=1)
        if(HARDWARE_MOD_16MB_FLASH)
            set(SDKCONFIG hardware/common/sdkconfig/ESP32/with_PSRAM/16MB_Flash)
        elseif(HARDWARE_MOD_8MB_FLASH)
            set(SDKCONFIG hardware/common/sdkconfig/ESP32/with_PSRAM/8MB_Flash)
        else()
            set(SDKCONFIG hardware/common/sdkconfig/ESP32/with_PSRAM/4MB_Flash)
        endif()
    else()        
        message(DEPRECATION
            "\n"
            "Target is missing PSRAM and is not officially supported due to known performance and functionality issues."
            "\n")
    endif()
endif()

if(USE_FAT_INSTEAD_OF_LITTLEFS)
    add_compile_options(-DESP3D_FATFS_FEATURE=1)
    else()
    add_compile_options(-DESP3D_LITTLEFS_FEATURE=1)
endif()

if(TARGET_FW_MARLIN)
    if(TFT_UI_SERVICE)
        add_compile_options("-I${CMAKE_SOURCE_DIR}/main/display/3dprinter/marlin")
    endif()
    add_compile_options("-I${CMAKE_SOURCE_DIR}/main/target/3dprinter/marlin")
elseif(TARGET_FW_REPETIER)
     if(TFT_UI_SERVICE)
        add_compile_options("-I${CMAKE_SOURCE_DIR}/main/display/3dprinter/repetier")
    endif()
    add_compile_options("-I${CMAKE_SOURCE_DIR}/main/target/3dprinter/repetier")
elseif(TARGET_FW_SMOOTHIEWARE)
     if(TFT_UI_SERVICE)
        add_compile_options("-I${CMAKE_SOURCE_DIR}/main/display/3dprinter/smoothiware")
    endif()
    add_compile_options("-I${CMAKE_SOURCE_DIR}/main/target/3dprinter/smoothiware")
elseif(TARGET_FW_GRBL)
     if(TFT_UI_SERVICE)
        add_compile_options("-I${CMAKE_SOURCE_DIR}/main/display/cnc/grbl")
    endif()
    add_compile_options("-I${CMAKE_SOURCE_DIR}/main/target/cnc/grbl")
else()
    message(FATAL_ERROR
        "\n"
        "No firmware target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
endif()


include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ESP3D-TFT specific log level
# All  = 2
# Error only = 1
# Disabled = 0
add_compile_options(-DESP3D_TFT_LOG=0)
# Disable ANSI color to fit some serial terminals 
add_compile_options(-DDISABLE_COLOR_LOG=0)


# ESP3D-TFT specific bechmark
# Enabled  = 1
# Disabled = 0
add_compile_options(-DESP3D_TFT_BENCHMARK=0)

if(ESP3D_AUTHENTICATION)
    add_compile_options(-DESP3D_AUTHENTICATION_FEATURE=1)
endif()

if(DISABLE_SERIAL_AUTHENTICATION)
    add_compile_options(-DESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE=1)
endif()

if(TIMESTAMP_ON_FILES)
    add_compile_options(-DESP3D_TIMESTAMP_FEATURE=1)
endif()

if(WIFI_SERVICE)
    add_compile_options(-DESP3D_WIFI_FEATURE=1)

    if(SSDP_SERVICE)
        add_compile_options(-DESP3D_SSDP_FEATURE=1)
    endif()

    if(MDNS_SERVICE)
        add_compile_options(-DESP3D_MDNS_FEATURE=1)
    endif()

    if(WEB_SERVICES)
        add_compile_options(-DESP3D_HTTP_FEATURE=1)

        if(WS_SERVICE)
            add_compile_options(-DESP3D_WS_SERVICE_FEATURE=1)
        endif()
    endif()

    if(TELNET_SERVICE)
        add_compile_options(-DESP3D_TELNET_FEATURE=1)
    endif()

    if(NOTIFICATIONS_SERVICE)
        add_compile_options(-DESP3D_NOTIFICATIONS_FEATURE=1)
    endif()
endif()

if(BT_SERVICE)
    add_compile_options(-DESP3D_BLUETOOTH_FEATURE=1)
endif()

if(TFT_UI_SERVICE)
    add_compile_options(-DESP3D_DISPLAY_FEATURE=1)
    # For lvgl_conf to use simple path
    add_compile_options(-DLV_CONF_INCLUDE_SIMPLE=1)
    add_compile_options(-DLV_LVGL_H_INCLUDE_SIMPLE=1)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/main/display")
    else()
    add_compile_options(-DLV_CONF_SUPPRESS_DEFINE_CHECK=1)
endif()

if(SD_CARD_SERVICE)
    add_compile_options(-DESP3D_SD_CARD_FEATURE=1)
    if(UPDATE_SERVICE)
        add_compile_options(-DESP3D_UPDATE_FEATURE=1)
    endif()
else()
unset(UPDATE_SERVICE CACHE)
endif()

if(GCODE_HOST_SERVICE)
    add_compile_options(-DESP3D_GCODE_HOST_FEATURE=1)
endif()

# Add entry point for customizations headers
add_compile_options("-I${CMAKE_SOURCE_DIR}/customizations")

add_compile_options(-DTFT_TARGET="${TFT_TARGET}")
project(ESP3D-TFT
    VERSION 1.0
    DESCRIPTION "ESP3D TFT")
message(STATUS "")
message(STATUS "----Project definition----")
message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
message(STATUS "TFT_TARGET = ${TFT_TARGET}")
message(STATUS "IDF_TARGET = ${IDF_TARGET}")
message(STATUS "--------------------------")
message(STATUS "")
