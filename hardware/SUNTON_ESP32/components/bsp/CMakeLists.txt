# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl spi_bus st7796 ili9341 sw_spi xpt2046 i2c_bus gt911 disp_backlight
)