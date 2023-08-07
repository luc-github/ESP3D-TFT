# Component directive
if(MDNS_SERVICE)
    set(mdnslib mdns)
endif()

idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES ${mdnslib} esp3d_log 
)