# main application directives

set(SOURCES .
    core core/esp3dcommands
    modules/authentication
    modules/serial
    modules/network
    modules/http
    modules/http/handlers
    modules/websocket
    modules/filesystem
    modules/filesystem/sd
    modules/filesystem/flash
    modules/update
    modules/notifications
    modules/mDNS
    modules/socket_server
    modules/usb_serial
    modules/gcode_host
)

if(SSDP_SERVICE)
    list(APPEND SOURCES "modules/ssdp")
endif()

unset(SSDP_SERVICE CACHE)

set(INCLUDES .
    core/includes
    modules)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    EMBED_FILES "embedded/favicon.ico.gz" "embedded/index.html.gz")
