# Component directive
if(USB_SERIAL_SERVICE)
set(USB_SERIAL " usb_serial")
else()
set(USB_SERIAL "")
endif()
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES "esp3d_log lvgl i2c_bus ft5x06 rm68120 tca9554 " ${USB_SERIAL}
)