if(ESP32_ROTRICS_DEXARM35)
    set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")    
    set(SDKCONFIG ${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/sdkconfig)
    set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components)
    #list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components/bsp")
endif()