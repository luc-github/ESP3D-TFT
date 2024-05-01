# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES "esp3d_log lvgl i2c_bus ft5x06 rm68120 tca9554 usb_serial"
)