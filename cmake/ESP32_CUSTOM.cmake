set(TFT_TARGET "ESP32_CUSTOM")
set(SDKCONFIG hardware/ESP32_CUSTOM/sdkconfig)
set(EXTRA_COMPONENT_DIRS hardware/ESP32_CUSTOM/components)
add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32_CUSTOM/components/bsp")