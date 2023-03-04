# main application directives

set(SOURCES .
    core core/esp3dcommands
    modules/authentication
    modules/serial
    modules/network
    modules/filesystem
    modules/filesystem/flash
    modules/usb_serial
)

if(WIFI_SERVICE)
    # TODO
endif()

if(SSDP_SERVICE)
    list(APPEND SOURCES "modules/ssdp")
endif()

if(MDNS_SERVICE)
    list(APPEND SOURCES "modules/mDNS")
endif()

if(BT_SERVICE)
    # TODO
endif()

if(WEB_SERVICES)
    list(APPEND SOURCES "modules/http")
    list(APPEND SOURCES "modules/http/handlers")
    list(APPEND SOURCES "modules/websocket")
endif()

if(TELNET_SERVICE)
    list(APPEND SOURCES "modules/socket_server")
endif()

if(SD_CARD_SERVICE)
    list(APPEND SOURCES "modules/filesystem/sd")
endif()

if(NOTIFICATIONS_SERVICE)
    list(APPEND SOURCES "modules/notifications")
endif()

if(UPDATE_SERVICE)
    list(APPEND SOURCES "modules/update")
endif()

if(GCODE_HOST_SERVICE)
    list(APPEND SOURCES "modules/gcode_host")
endif()

unset(WIFI_SERVICE CACHE)
unset(SSDP_SERVICE CACHE)
unset(MDNS_SERVICE CACHE)
unset(BT_SERVICE CACHE)
unset(WEB_SERVICES CACHE)
unset(TELNET_SERVICE CACHE)
unset(WS_SERVICE CACHE)
unset(SD_CARD_SERVICE CACHE)
unset(NOTIFICATIONS_SERVICE CACHE)
unset(UPDATE_SERVICE CACHE)
unset(GCODE_HOST_SERVICE CACHE)

set(INCLUDES .
    core/includes
    modules)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    EMBED_FILES "embedded/favicon.ico.gz" "embedded/index.html.gz")
