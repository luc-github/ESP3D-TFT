# #######################################
# Select the target
# #######################################
OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R 2.8inches" OFF)
OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" ON)
OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF)
OPTION(ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" OFF)

# #######################################
# Select the Options
# #######################################
OPTION(ESP3D_AUTHENTICATION "Authentication on all clients" OFF)
OPTION(DISABLE_SERIAL_AUTHENTICATION "Disable Serial Authentication" OFF)
OPTION(TIMESTAMP_ON_FILES "Time on files" ON)

# #######################################
# Do not change anything above this line
# #######################################
cmake_minimum_required(VERSION 3.12.4)
set(CMAKE_CXX_STANDARD 20)

if(ESP32_2432S028R)
    set(SDKCONFIG targets/ESP32_2432S028R/sdkconfig)
    set(EXTRA_COMPONENT_DIRS targets/ESP32_2432S028R/components)
elseif(ESP32S3_HMI43V3)
    set(SDKCONFIG targets/ESP32S3_HMI43V3/sdkconfig)
    set(EXTRA_COMPONENT_DIRS targets/ESP32S3_HMI43V3/components)
elseif(ESP32_ROTRICS_DEXARM35)
    set(SDKCONFIG targets/ESP32_ROTRICS_DEXARM35/sdkconfig)
    set(EXTRA_COMPONENT_DIRS targets/ESP32_ROTRICS_DEXARM35/components)
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    set(SDKCONFIG targets/ESP32S3_ZX3D50CE02S_USRC_4832/sdkconfig)
    set(EXTRA_COMPONENT_DIRS targets/ESP32S3_ZX3D50CE02S_USRC_4832/components)
else()
    message(FATAL_ERROR
        "\n"
        "No target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# For lvgl_conf to use simple path
add_compile_options(-DLV_CONF_INCLUDE_SIMPLE=1)
add_compile_options(-DLV_LVGL_H_INCLUDE_SIMPLE=1)

# ESP3D-TFT specific log level
# Enabled  = 1
# Disabled = 0
add_compile_options(-DESP3D_TFT_LOG=1)

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

if(ESP32_2432S028R)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/targets/ESP32_2432S028R/components/bsp")
    set(EXTRA_COMPONENT_DIRS targets/ESP32_2432S028R/components)
    set(TFT_TARGET "ESP32_2432S028R")
elseif(ESP32S3_HMI43V3)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/targets/ESP32S3_HMI43V3/components/bsp")
    set(EXTRA_COMPONENT_DIRS targets/ESP32S3_HMI43V3/components)
    set(TFT_TARGET "ESP32S3_HMI43V3")
elseif(ESP32_ROTRICS_DEXARM35)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/targets/ESP32_ROTRICS_DEXARM35/components/bsp")
    set(EXTRA_COMPONENT_DIRS targets/ESP32_ROTRICS_DEXARM35/components)
    set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/targets/ESP32S3_ZX3D50CE02S_USRC_4832/components/bsp")
    set(EXTRA_COMPONENT_DIRS targets/ESP32S3_ZX3D50CE02S_USRC_4832/components)
    set(TFT_TARGET "ESP32S3_ZX3D50CE02S_USRC_4832")
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
