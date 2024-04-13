# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl i2c_bus spi_bus gt911 st7796 disp_backlight camera
)
#usb_serial 