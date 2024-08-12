# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl esp_lcd i2c_bus st7262 gt911 usb_serial
)