# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl spi_bus st7796 xpt2046 disp_backlight
)