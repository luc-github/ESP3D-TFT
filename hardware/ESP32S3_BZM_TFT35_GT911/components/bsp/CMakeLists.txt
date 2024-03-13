# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES usb_serial esp3d_log lvgl spi_bus i2c_bus st7796 gt911 disp_backlight
)