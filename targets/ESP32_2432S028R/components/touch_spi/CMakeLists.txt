# Component directive
set(SOURCES "touch_spi.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
)
