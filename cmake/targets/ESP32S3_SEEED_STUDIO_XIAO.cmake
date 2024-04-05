if(ESP32S3_SEEED_STUDIO_XIAO)
    set(TFT_TARGET "ESP32S3_SEEED_STUDIO_XIAO")
    set(SDKCONFIG ${CMAKE_SOURCE_DIR}/hardware/std::ESP32S3_SEEED_STUDIO_XIAO/sdkconfig)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/std::ESP32S3_SEEED_STUDIO_XIAO/components)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/std::ESP32S3_SEEED_STUDIO_XIAO/components/bsp")    
    if (USB_SERIAL_SERVICE)
        # Enable USB-OTG as serial alternative for communications
        add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
        # Add specific usb driver for otg
        list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_usb_otg)
    endif()
    unset(TFT_UI_SERVICE CACHE)
endif()