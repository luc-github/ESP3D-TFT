# Component directive
set(SOURCES "ft5x06.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES esp3d_log lvgl i2c_bus
)