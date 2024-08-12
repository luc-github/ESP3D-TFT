if(ESP32S3_ZX3D50CE02S_USRC_4832)
    set(TFT_TARGET "ESP32S3_ZX3D50CE02S_USRC_4832")
    set(SDKCONFIG ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/sdkconfig)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_video_i80)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_ZX3D50CE02S_USRC_4832/components/bsp")
    # Add specific usb driver for otg
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_usb_otg)    
    if (USB_SERIAL_SERVICE)
        # Enable USB-OTG as serial alternative for communications
        add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
    endif()
    set (RESOLUTION_SCREEN "res_480_320")
endif()
