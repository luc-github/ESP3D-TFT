if(ESP32S3_BZM_TFT35_GT911)
    set(TFT_TARGET "ESP32S3_BZM_TFT35_GT911")
    add_compile_options(-DWITH_PSRAM=1)
    #add_compile_options(-DWITH_GT911_INT=1)
    set(SDKCONFIG ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_BZM_TFT35_GT911/sdkconfig)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_BZM_TFT35_GT911/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_BZM_TFT35_GT911/components/bsp")
    if (USB_SERIAL_SERVICE)
        # Enable USB-OTG as serial alternative for communications
        add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
        # Add specific usb driver for otg
        list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_usb_otg)
    endif()
endif()