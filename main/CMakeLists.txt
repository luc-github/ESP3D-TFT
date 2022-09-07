set(SOURCES main.cpp core/esp3d-tft.cpp)
set(INCLUDES . core/includes)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
