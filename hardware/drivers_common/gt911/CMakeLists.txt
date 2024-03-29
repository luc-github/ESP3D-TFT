# Component directive
set(SOURCES "gt911.c")
set(INCLUDES .)
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES esp3d_log i2c_bus
)