# main application directives
set(SOURCES . core core/esp3dcommands
    modules/authentication
    modules/serial
    modules/network
    modules/filesystem
    modules/filesystem/sd
    modules/filesystem/flash
)
set(INCLUDES . core/includes modules)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
