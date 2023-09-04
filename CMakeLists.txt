# #######################################
# Select the targeted hardware
# #######################################

OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" ON)
OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF)
OPTION(ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" OFF)
OPTION(ESP32S3_8048S070 "TFT TARGET is ESP32S3 7.0inches" OFF)
OPTION(ESP32S3_8048S050 "TFT TARGET is ESP32S3 5.0inches" OFF)
OPTION(ESP32S3_8048S043 "TFT TARGET is ESP32S3 4.3inches (800x480)" OFF)
OPTION(ESP32S3_4827S043 "TFT TARGET is ESP32S3 4.3inches (480x272)" OFF)
OPTION(ESP32S3_FREENOVE_1_1 "HARDWARE TARGET is ESP32S3 Freenove v1.1" OFF)
OPTION(ESP32S3_CUSTOM "HARDWARE TARGET is ESP32S3 custom board" OFF)
OPTION(ESP32_CUSTOM "HARDWARE TARGET is ESP32 custom board" OFF)

# WARNING:
#   Limited help or support available for these targets.
#   PSRAM hardware mod is REQUIRED for full functionality and performance.
OPTION(ESP32_3248S035R "TFT TARGET is ESP32_3248S035R 3.5inches (Resistive)" OFF)
OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R 2.8inches (Resistive)" OFF)


# #######################################
# Optionally select a hardware variant
# #######################################
if(ESP32_3248S035R)
    OPTION(ESP32_3248S035R_ORIGINAL "ESP32 3.5inches (Resistive)  - Original hardware with 4MB Flash, without PSRAM" ON)
    OPTION(ESP32_3248S035R_4MB_WITH_PSRAM "ESP32 3.5inches (Resistive) - 4MB Flash, with External PSRAM mod" OFF)
    OPTION(ESP32_3248S035R_16MB_WITH_PSRAM "ESP32 3.5inches (Resistive) - 16MB Flash mod, with External PSRAM mod" OFF)
elseif(ESP32_2432S028R)
    OPTION(ESP32_2432S028R_ORIGINAL "ESP32 2.8inches (Resistive) - Original hardware with 4MB Flash, without PSRAM" OFF)
    OPTION(ESP32_2432S028R_4MB_WITH_PSRAM "ESP32 2.8inches (Resistive) - 4MB Flash, with External PSRAM mod" OFF)
    OPTION(ESP32_2432S028R_16MB_WITH_PSRAM "ESP32 2.8inches (Resistive) - 16MB Flash mod, with External PSRAM mod" ON)
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
if(ESP32S3_FREENOVE_1_1)
    set(SDKCONFIG hardware/ESP32S3_FREENOVE_1_1/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_FREENOVE_1_1/components)
    unset(TFT_UI_SERVICE CACHE)
elseif(ESP32S3_8048S070)
    set(SDKCONFIG hardware/ESP32S3_8048S070/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S070/components)
elseif(ESP32S3_8048S050)
    set(SDKCONFIG hardware/ESP32S3_8048S050/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S050/components)
elseif(ESP32S3_8048S043)
    set(SDKCONFIG hardware/ESP32S3_8048S043/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S043/components)
elseif(ESP32S3_4827S043)
    set(SDKCONFIG hardware/ESP32S3_4827S043/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_4827S043/components)
elseif(ESP32_3248S035R)
    if(ESP32_3248S035R_16MB_WITH_PSRAM)
        set(SDKCONFIG hardware/ESP32_3248S035R/variants/16MB_with_PSRAM/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035R/variants/16MB_with_PSRAM/components)
        add_compile_options(-DWITH_PSRAM=1)
    elseif(ESP32_3248S035R_4MB_WITH_PSRAM)
        set(SDKCONFIG hardware/ESP32_3248S035R/variants/4MB_with_PSRAM/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035R/variants/4MB_with_PSRAM/components)
        add_compile_options(-DWITH_PSRAM=1)
    else()
        set(SDKCONFIG hardware/ESP32_3248S035R/variants/4MB_original/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035R/variants/4MB_original/components)
        message(DEPRECATION
            "\n"
            "Target is missing PSRAM and is not officially supported due to known performance and functionality issues."
            "\n")
    endif()
elseif(ESP32_2432S028R)
    if(ESP32_2432S028R_16MB_WITH_PSRAM)
        set(SDKCONFIG hardware/ESP32_2432S028R/variants/16MB_with_PSRAM/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_2432S028R/variants/16MB_with_PSRAM/components)
        add_compile_options(-DWITH_PSRAM=1)
    elseif(ESP32_2432S028R_4MB_WITH_PSRAM)
        set(SDKCONFIG hardware/ESP32_2432S028R/variants/4MB_with_PSRAM/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_2432S028R/variants/4MB_with_PSRAM/components)
        add_compile_options(-DWITH_PSRAM=1)
    else()
        set(SDKCONFIG hardware/ESP32_2432S028R/variants/4MB_original/sdkconfig)
        set(EXTRA_COMPONENT_DIRS hardware/ESP32_2432S028R/variants/4MB_original/components)
        message(DEPRECATION
            "\n"
            "Target is missing PSRAM and is not officially supported due to known performance and functionality issues."
            "\n")
    endif()
elseif(ESP32S3_HMI43V3)
    set(SDKCONFIG hardware/ESP32S3_HMI43V3/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_HMI43V3/components)
elseif(ESP32_ROTRICS_DEXARM35)
    set(SDKCONFIG hardware/ESP32_ROTRICS_DEXARM35/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_ROTRICS_DEXARM35/components)
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    set(SDKCONFIG hardware/ESP32S3_ZX3D50CE02S_USRC_4832/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components)
elseif(ESP32S3_CUSTOM)
    set(SDKCONFIG hardware/ESP32S3_CUSTOM/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_CUSTOM/components)
elseif(ESP32_CUSTOM)
    set(SDKCONFIG hardware/ESP32_CUSTOM/sdkconfig)
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_CUSTOM/components)
else()
    message(FATAL_ERROR
        "\n"
        "No hardware target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
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
add_compile_options(-DESP3D_TFT_LOG=2)
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

if(ESP32_2432S028R)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_2432S028R/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_2432S028R/components)
    set(TFT_TARGET "ESP32_2432S028R")
elseif(ESP32_3248S035R)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_3248S035R/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_3248S035R/components)
    set(TFT_TARGET "ESP32_3248S035R")
elseif(ESP32S3_HMI43V3)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_HMI43V3/components)
    set(TFT_TARGET "ESP32S3_HMI43V3")
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32S3_8048S070)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S070/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S070/components)
    set(TFT_TARGET "ESP32S3_8048S070")
elseif(ESP32S3_8048S050)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S050/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S050/components)
    set(TFT_TARGET "ESP32S3_8048S050")
elseif(ESP32S3_8048S043)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_8048S043/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_8048S043/components)
    set(TFT_TARGET "ESP32S3_8048S043")
elseif(ESP32S3_4827S043)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_4827S043/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_4827S043/components)
    set(TFT_TARGET "ESP32S3_4827S043")
elseif(ESP32_ROTRICS_DEXARM35)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_ROTRICS_DEXARM35/components)
    set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components)
    set(TFT_TARGET "ESP32S3_ZX3D50CE02S_USRC_4832")
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32S3_FREENOVE_1_1)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_FREENOVE_1_1/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_FREENOVE_1_1/components)
    set(TFT_TARGET "ESP32S3_FREENOVE_1_1")
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32S3_CUSTOM)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_CUSTOM/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_CUSTOM/components)
    set(TFT_TARGET "ESP32S3_CUSTOM")
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
elseif(ESP32_CUSTOM)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_CUSTOM/components/bsp")
    set(EXTRA_COMPONENT_DIRS hardware/ESP32_CUSTOM/components)
    set(TFT_TARGET "ESP32_CUSTOM")
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
