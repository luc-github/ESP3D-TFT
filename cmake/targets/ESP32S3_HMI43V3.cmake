# Ensure the board is enabled in CMakeLists.txt
if(ESP32S3_HMI43V3)
    # Add board name
    set(TFT_TARGET "ESP32S3_HMI43V3")
    # Add the sdkconfig file path    
    set(SDKCONFIG  ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/sdkconfig)
    # Add Specific Components if any for the board
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/components)
    # Add specific video driver for i80
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_video_i80)
    # Add specific bsp path for board definition 
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/ESP32S3_HMI43V3/components/bsp")
    if (USB_SERIAL_SERVICE)
        # Enable USB-OTG as serial alternative for communications
        add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
        # Add specific usb driver for otg
        list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_usb_otg)
    endif()
endif()