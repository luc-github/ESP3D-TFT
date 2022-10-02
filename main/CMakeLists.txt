# main application directives
set(SOURCES main.cpp core/esp3d-tft.cpp core/esp3d-tft-ui.cpp core/esp3d-tft-stream.cpp core/esp3d-tft-network.cpp)
set(INCLUDES . core/includes)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
