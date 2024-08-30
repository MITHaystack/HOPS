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

set(EXTERNAL_LIBLIST
    PkgConfig::DIFXIO
    m
)

# target_link_libraries(${EXENAME} PkgConfig::DIFXIO ${EXTERNAL_LIBLIST})

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME}  ${EXTERNAL_LIBLIST} ${INTERNAL_LIBLIST} )

#we do not install difx2mark4 headers
#hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
