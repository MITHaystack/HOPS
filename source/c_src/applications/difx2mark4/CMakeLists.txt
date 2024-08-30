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

# #mimic a bunch of checks automake does (which AFAIK are not used anywhere)
# include(CheckIncludeFile)
# include(CheckIncludeFiles)
# check_include_file(dlfcn.h HAVE_DLFCN_H)
# check_include_file(inttypes.h HAVE_INTTYPES_H)
# check_include_file(memory.h HAVE_MEMORY_H)
# check_include_file(stdint.h HAVE_STDINT_H)
# check_include_file(stdlib.h HAVE_STDLIB_H)
# check_include_file(strings.h HAVE_STRINGS_H)
# check_include_file(string.h HAVE_STRING_H)
# check_include_file(sys/stat.h HAVE_SYS_STAT_H)
# check_include_file(sys/types.h HAVE_SYS_TYPES_H)
# check_include_file(unistd.h HAVE_UNISTD_H)
# check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

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

# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/d2m4_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/d2m4_config.h)
# hops_install_headers(${CMAKE_CURRENT_BINARY_DIR}/d2m4_config.h)
# hops_include_directories(${CMAKE_CURRENT_BINARY_DIR})


#declare exe name ############################################################
set(EXENAME "difx2mark4")

#list header files #############################################################

file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
endforeach( BASENAME )

# #add the d2m4_config.h file
# list( APPEND ${EXENAME}_HEADERFILES ${CMAKE_CURRENT_BINARY_DIR}/d2m4_config.h)

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
# set(VEX_RELPATH "../../vex")


list(APPEND RELPATH_LIST
    # ${MSG_RELPATH}
    ${MK4UTIL_RELPATH}
    # ${AFIO_RELPATH}
    ${DFIO_RELPATH}
    # ${VEX_RELPATH}
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
    # "afio"
    "dfio"
    # "vex"
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


















#
#
#
#
#
# cmake_minimum_required(VERSION 3.0)
# project(difx2mark4)
#
# if(POLICY CMP0053)
#     cmake_policy(SET CMP0053 OLD)
# endif()
#
# # Module version
# set(MODULE_VERSION_MAJOR 0)
# set(MODULE_VERSION_MINOR 0)
# set(MODULE_VERSION_PATCH 0)
# set(MODULE_VERSION "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH}")
# set(MODULE_VERSION_NUMBER "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}${MODULE_VERSION_PATCH}")
#
# set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
# include( difx2mark4Defaults )
#
# if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#     set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "default install path" FORCE )
# endif()
#
#
#
# #define install directories
# set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
# set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
# set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
# set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/config")
# set(SHARE_TEXT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/text")
# set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/data")
#
# file(MAKE_DIRECTORY ${INCLUDE_INSTALL_DIR})
# file(MAKE_DIRECTORY ${LIB_INSTALL_DIR})
# file(MAKE_DIRECTORY ${BIN_INSTALL_DIR})
# file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
# file(MAKE_DIRECTORY ${DATA_INSTALL_DIR})
# file(MAKE_DIRECTORY ${SHARE_TEXT_INSTALL_DIR})
#
# difx2mark4_module_paths( difx2mark4 )
#
# #compiler flags
# set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
# set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )
# message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION}")
#
# # #set source directories
# # set (CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
# # set (CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
# # set (PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)
# # set (SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
#
# #config header generation to mimic behavior of autoheader generated hops_config.h
# #some of these are probably not necessary
# #
# # include(TestBigEndian)
# # TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
# # if(IS_BIG_ENDIAN)
# #     message(STATUS "Machine is BIG_ENDIAN")
# #     set(WORDS_BIGENDIAN TRUE)
# # else()
# #     message(STATUS "Machine is LITTLE_ENDIAN")
# #     set(WORDS_BIGENDIAN FALSE)
# #     # difx2mark4_add_cflag(BYTFLP)
# # endif()
# #
# #mimic a bunch of checks automake does (which afaik are not used anywhere)
# include(CheckIncludeFile)
# include(CheckIncludeFiles)
# check_include_file(dlfcn.h HAVE_DLFCN_H)
# check_include_file(inttypes.h HAVE_INTTYPES_H)
# check_include_file(memory.h HAVE_MEMORY_H)
# check_include_file(stdint.h HAVE_STDINT_H)
# check_include_file(stdlib.h HAVE_STDLIB_H)
# check_include_file(strings.h HAVE_STRINGS_H)
# check_include_file(string.h HAVE_STRING_H)
# check_include_file(sys/stat.h HAVE_SYS_STAT_H)
# check_include_file(sys/types.h HAVE_SYS_TYPES_H)
# check_include_file(unistd.h HAVE_UNISTD_H)
# check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)
#
# set(HOPS_SVN_REV_VALUE 0)
# set(HOPS_REVISION_VALUE ${MODULE_VERSION_NUMBER})
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/difxio_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)
# difx2mark4_install_headers(${CMAKE_CURRENT_BINARY_DIR}/include/config.h)
# difx2mark4_include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
# difx2mark4_add_cflag(HAVE_CONFIG_H)
#
# #mk4 and vex libraries
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/difxio)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/difx2mark4)
#
# #configure env script
# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/difx2mark4env.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/difx2mark4env.sh @ONLY)
# install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/difx2mark4env.sh DESTINATION ${BIN_INSTALL_DIR} )
