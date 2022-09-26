# Component directive
idf_component_register(SRCS "lvgl_helpers.c"
    INCLUDE_DIRS .
    REQUIRES lvgl display touch
)

target_compile_definitions("${COMPONENT_LIB}" PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)