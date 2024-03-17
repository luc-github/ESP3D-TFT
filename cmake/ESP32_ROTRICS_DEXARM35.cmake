set(TFT_TARGET "ESP32_ROTRICS_DEXARM35")    
set(SDKCONFIG hardware/ESP32_ROTRICS_DEXARM35/sdkconfig)
set(EXTRA_COMPONENT_DIRS hardware/ESP32_ROTRICS_DEXARM35/components)
add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_ROTRICS_DEXARM35/components/bsp")