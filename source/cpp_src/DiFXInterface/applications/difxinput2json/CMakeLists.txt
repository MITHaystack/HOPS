# difxinput2json difx .input to JSON converter, links against difxio

hops_internal_include_directories()
hops_internal_include_directories(${EXTERN_HLIBS}/cli11)

# we use pkg-config to find difxio
find_package(PkgConfig)
pkg_search_module(DIFXIO REQUIRED difxio IMPORTED_TARGET)
hops_external_include_directories(${DIFXIO_INCLUDE_DIRS})
message(STATUS "difxinput2json: found difxio version: ${DIFXIO_VERSION}")

# Probe for difx_options.h (showed up around DiFX 2.8.1). Without it the
# --localdir option is unavailable.
include(CheckIncludeFile)
set(CMAKE_REQUIRED_INCLUDES ${DIFXIO_INCLUDE_DIRS})
unset(HAVE_DIFX_OPTIONS_H CACHE)
check_include_file("difxio/difx_options.h" HAVE_DIFX_OPTIONS_H)
unset(CMAKE_REQUIRED_INCLUDES)

if(HAVE_DIFX_OPTIONS_H)
    message(STATUS "difxinput2json: found difx_options.h, will enable --localdir option")
    add_compile_definitions(HAVE_DIFX_OPTS)
else()
    message(STATUS "difxinput2json: difx_options.h not found, --localdir option is unavailable")
endif()

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(DIFXINPUT2JSON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/difxinput2json.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/MHO_DiFXInputProcessor.cc
)

add_executable(difxinput2json ${DIFXINPUT2JSON_SOURCES})

# Try to STATICALLY link difxio, that way this (GPL) binary is self-contained,
# and carries no runtime libdifxio.so dependency and needs no difxio RPATH,
# making it fully relocatable. difxio's only external dependency is libm.
# We force the static (.a) suffix when searching for the lib so we don't accidentally match
# the shared library, then reset it when done
set(_saved_find_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(DIFXIO_STATIC_LIB
    NAMES difxio
    HINTS ${DIFXIO_LIBDIR} ${DIFXIO_LIBRARY_DIRS}
)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_find_suffixes})

if(DIFXIO_STATIC_LIB)
    message(STATUS "difxinput2json: static-linking difxio from ${DIFXIO_STATIC_LIB}")
    target_link_libraries(difxinput2json MHO_Message MHO_Utilities ${DIFXIO_STATIC_LIB} m)
    # libdifxio.a may be compiled without -fPIC, however our
    # toolchain defaults to PIE. So we build just this target as non-PIE in order to link it in.
    # That means this binary skips PIE/ASLR hardening... but that is honestly not important here
    set_target_properties(difxinput2json PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    target_link_options(difxinput2json PRIVATE -no-pie)
    install(TARGETS difxinput2json RUNTIME DESTINATION ${BIN_INSTALL_RELDIR})
else()
    # Fallback: no static archive available, link the shared difxio. In this case
    # we need INSTALL_RPATH_USE_LINK_PATH so libdifxio.so resolves at runtime, but will not be relocatable.
    # DEV NOTE: Do not generate binaries for distribution in this case!
    message(STATUS "difxinput2json: static libdifxio.a not found, linking shared difxio")
    target_link_libraries(difxinput2json MHO_Message MHO_Utilities PkgConfig::DIFXIO)
    set_target_properties(difxinput2json PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON)
    install(TARGETS difxinput2json RUNTIME DESTINATION ${BIN_INSTALL_RELDIR})
endif()
