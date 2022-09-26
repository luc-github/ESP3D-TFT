# Component directive
idf_component_register(SRCS "touch_driver.c" "tp_spi.c" "xpt2046.c"
    INCLUDE_DIRS .
    REQUIRES lvgl lvgl_conf display
)

target_compile_definitions("${COMPONENT_LIB}" PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)