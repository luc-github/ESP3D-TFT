# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl spi_bus disp_spi ili9341 sw_spi xpt2046 disp_backlight
)