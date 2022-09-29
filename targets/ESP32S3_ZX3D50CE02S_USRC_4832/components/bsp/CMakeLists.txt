# Component directive
idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES lvgl i2c_bus ft5x06 st7796
)