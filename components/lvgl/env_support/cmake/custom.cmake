# Option to define LV_LVGL_H_INCLUDE_SIMPLE, default: ON
option(LV_LVGL_H_INCLUDE_SIMPLE
       "Use #include \"lvgl.h\" instead of #include \"../../lvgl.h\"" ON)

# Option to define LV_CONF_INCLUDE_SIMPLE, default: ON
option(LV_CONF_INCLUDE_SIMPLE
       "Use #include \"lv_conf.h\" instead of #include \"../../lv_conf.h\"" ON)

# Option LV_CONF_PATH, which should be the path for lv_conf.h
# If set parent path LV_CONF_DIR is added to includes
if( LV_CONF_PATH )
    get_filename_component(LV_CONF_DIR ${LV_CONF_PATH} DIRECTORY)
endif( LV_CONF_PATH )

# Option to build shared libraries (as opposed to static), default: OFF
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Set sources used for LVGL components
file(GLOB_RECURSE SOURCES ${LVGL_ROOT_DIR}/src/*.c ${LVGL_ROOT_DIR}/src/*.S)
file(GLOB_RECURSE EXAMPLE_SOURCES ${LVGL_ROOT_DIR}/examples/*.c)
file(GLOB_RECURSE DEMO_SOURCES ${LVGL_ROOT_DIR}/demos/*.c)
file(GLOB_RECURSE THORVG_SOURCES ${LVGL_ROOT_DIR}/src/libs/thorvg/*.cpp ${LVGL_ROOT_DIR}/src/others/vg_lite_tvg/*.cpp)

# Build LVGL library
add_library(lvgl ${SOURCES})
add_library(lvgl::lvgl ALIAS lvgl)

target_compile_definitions(
  lvgl PUBLIC $<$<BOOL:${LV_LVGL_H_INCLUDE_SIMPLE}>:LV_LVGL_H_INCLUDE_SIMPLE>
              $<$<BOOL:${LV_CONF_INCLUDE_SIMPLE}>:LV_CONF_INCLUDE_SIMPLE>
              $<$<COMPILE_LANGUAGE:ASM>:__ASSEMBLY__>)

# Add definition of LV_CONF_PATH only if needed
if(LV_CONF_PATH)
  target_compile_definitions(lvgl PUBLIC LV_CONF_PATH=${LV_CONF_PATH})
endif()

# Include root and optional parent path of LV_CONF_PATH
target_include_directories(lvgl SYSTEM PUBLIC ${LVGL_ROOT_DIR} ${LV_CONF_DIR})


if(NOT LV_CONF_BUILD_DISABLE_THORVG_INTERNAL)
    add_library(lvgl_thorvg ${THORVG_SOURCES})
    add_library(lvgl::thorvg ALIAS lvgl_thorvg)
    target_include_directories(lvgl_thorvg SYSTEM PUBLIC ${LVGL_ROOT_DIR}/src/libs/thorvg)
    if(LV_CONF_PATH)
        target_compile_definitions(lvgl_thorvg PUBLIC LV_CONF_PATH=${LV_CONF_PATH})
    endif()
endif()

# Build LVGL example library
if(NOT LV_CONF_BUILD_DISABLE_EXAMPLES)
    add_library(lvgl_examples ${EXAMPLE_SOURCES})
    add_library(lvgl::examples ALIAS lvgl_examples)

    target_include_directories(lvgl_examples SYSTEM PUBLIC ${LVGL_ROOT_DIR}/examples)
    target_link_libraries(lvgl_examples PUBLIC lvgl)
endif()

# Build LVGL demos library
if(NOT LV_CONF_BUILD_DISABLE_DEMOS)
    add_library(lvgl_demos ${DEMO_SOURCES})
    add_library(lvgl::demos ALIAS lvgl_demos)

    target_include_directories(lvgl_demos SYSTEM PUBLIC ${LVGL_ROOT_DIR}/demos)
    target_link_libraries(lvgl_demos PUBLIC lvgl)
endif()

# Lbrary and headers can be installed to system using make install
file(GLOB LVGL_PUBLIC_HEADERS "${CMAKE_SOURCE_DIR}/lv_conf.h"
     "${CMAKE_SOURCE_DIR}/lvgl.h")

if("${LIB_INSTALL_DIR}" STREQUAL "")
  set(LIB_INSTALL_DIR "lib")
endif()
if("${RUNTIME_INSTALL_DIR}" STREQUAL "")
  set(RUNTIME_INSTALL_DIR "bin")
endif()
if("${INC_INSTALL_DIR}" STREQUAL "")
  set(INC_INSTALL_DIR "include/lvgl")
endif()

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/"
  FILES_MATCHING
  PATTERN "*.h")

configure_file("${LVGL_ROOT_DIR}/lvgl.pc.in" lvgl.pc @ONLY)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/lvgl.pc"
  DESTINATION "${LIB_INSTALL_DIR}/pkgconfig/")

set_target_properties(
  lvgl
  PROPERTIES OUTPUT_NAME lvgl
             ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             PUBLIC_HEADER "${LVGL_PUBLIC_HEADERS}")

install(
  TARGETS lvgl
  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
  RUNTIME DESTINATION "${RUNTIME_INSTALL_DIR}"
  PUBLIC_HEADER DESTINATION "${INC_INSTALL_DIR}")
