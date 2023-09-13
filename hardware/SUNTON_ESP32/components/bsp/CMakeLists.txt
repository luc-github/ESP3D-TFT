# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl spi_bus disp_spi ili9341 st7796 sw_spi xpt2046 i2c_bus gt911 disp_backlight
)