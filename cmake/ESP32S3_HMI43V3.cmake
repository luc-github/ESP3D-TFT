set(TFT_TARGET "ESP32S3_HMI43V3")    
set(SDKCONFIG hardware/ESP32S3_HMI43V3/sdkconfig)
set(EXTRA_COMPONENT_DIRS hardware/ESP32S3_HMI43V3/components)
add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/components/bsp")
add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)