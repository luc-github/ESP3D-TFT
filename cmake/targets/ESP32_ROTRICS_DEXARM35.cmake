if(ESP32_ROTRICS_DEXARM35)
    set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")    
    set(SDKCONFIG ${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/sdkconfig)
    set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components/bsp")
    set (RESOLUTION_SCREEN "res_480_320")
endif()