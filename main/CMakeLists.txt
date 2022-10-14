# main application directives
set(SOURCES . core modules/authentication modules/serial modules/filesystem modules/filesystem/sd)
set(INCLUDES . core/includes modules)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
