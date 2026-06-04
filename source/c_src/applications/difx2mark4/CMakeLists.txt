# pre-requisites (difxio) ###############################################################
# we use pkg-config to find difxio
find_package(PkgConfig)
pkg_search_module(DIFXIO REQUIRED difxio IMPORTED_TARGET)

#config header generation to mimic behavior of autoheader generated config.h
#most of these are probably not necessary
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(STATUS "Machine is BIG_ENDIAN")
    set(WORDS_BIGENDIAN TRUE)
else()
    message(STATUS "Machine is LITTLE_ENDIAN")
    set(WORDS_BIGENDIAN FALSE)
endif()

#add a flag to specify the difxio version
set(DIFXIO_VERSION_STRING "DIFXIO_VERSION=\\\\\"${DIFXIO_VERSION}\\\\\"")
hops_add_cflag(${DIFXIO_VERSION_STRING})

string(REPLACE "." ";" VERSION_LIST ${DIFXIO_VERSION})
list(GET VERSION_LIST 0 DIFXIO_VERSION_MAJOR)
list(GET VERSION_LIST 1 DIFXIO_VERSION_MINOR)
list(GET VERSION_LIST 2 DIFXIO_VERSION_PATCH)

#add flags to specify the numerical equivalents
hops_add_cflag("DIFXIO_VERSION_MAJOR=${DIFXIO_VERSION_MAJOR}")
hops_add_cflag("DIFXIO_VERSION_MINOR=${DIFXIO_VERSION_MINOR}")
hops_add_cflag("DIFXIO_VERSION_PATCH=${DIFXIO_VERSION_PATCH}")

#declare exe name ############################################################
set(EXENAME "difx2mark4")

#list header files #############################################################

file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
endforeach( BASENAME )

#list source files #############################################################

file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
endforeach( BASENAME )

list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/difx2mark4.c)

#declare relative paths to internal headers/libs ###############################
set(MK4UTIL_RELPATH "../../mk4util")
set(DFIO_RELPATH "../../dfio")

list(APPEND RELPATH_LIST
    ${MK4UTIL_RELPATH}
    ${DFIO_RELPATH}
)

#include internal library headers
foreach(path ${RELPATH_LIST})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include exe headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#include external library headers
hops_external_include_directories (${DIFXIO_INCLUDE_DIRS})

# set library lists ############################################################
set(INTERNAL_LIBLIST
    "mk4util"
    "dfio"
)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )

# Try to STATICALLY link difxio so this binary is self-contained and
# carries no runtime libdifxio.so dependency, (same as difxinput2json).
# difxio's only external dependency is libm. Force the static (.a) suffix when
# searching so we don't accidentally match the shared library, then restore it.
set(_saved_find_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(DIFXIO_STATIC_LIB NAMES difxio HINTS ${DIFXIO_LIBDIR} ${DIFXIO_LIBRARY_DIRS})
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_find_suffixes})

if(DIFXIO_STATIC_LIB)
    message(STATUS "difx2mark4: static-linking difxio from ${DIFXIO_STATIC_LIB}")
    target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${DIFXIO_STATIC_LIB} m)
    # libdifxio.a may be built without -fPIC while our toolchain defaults to PIE,
    # so build just this target as non-PIE in order to link it
    set_target_properties(${EXENAME} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    target_link_options(${EXENAME} PRIVATE -no-pie)
else()
    # In license-compat mode do not ship a non-relocatable (shared) difxio-linked
    # binary; error out so we don't distribute a broken bdist.
    if(HOPS_BDIST_LICENSE_COMPAT_ONLY)
        message(FATAL_ERROR
            "difx2mark4: static libdifxio.a not found. "
            "HOPS_BDIST_LICENSE_COMPAT_ONLY=ON requires a statically-linked, relocatable "
            "difx2mark4 for redistribution. "
            "Install a static libdifxio.a, or set -DHOPS3_ENABLE_BUILD_D2M4=OFF to omit difx2mark4.")
    endif()
    message(STATUS "difx2mark4: static libdifxio.a not found, linking shared difxio")
    target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} PkgConfig::DIFXIO m)
    set_target_properties(${EXENAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON)
endif()

#we do not install difx2mark4 headers
#hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
