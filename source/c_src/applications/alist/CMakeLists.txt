# if(POLICY CMP0053)
#     cmake_policy(SET CMP0053 OLD)
# endif()
# 
# include_directories(${CSOURCE}/mk4util/include)
# include_directories(${CSOURCE}/dfio/include)
# include_directories(${CSOURCE}/afio/include)
# 
# set (ALIST_LIBS
#     mk4util
#     dfio
#     afio
#     m
#   )
# 
# set (ALIST_SOURCEFILES
#     ${CMAKE_CURRENT_SOURCE_DIR}/alist.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/open_output.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/parse_cmdline.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/summarize_mk4fringe.c
# )
# 
# add_executable (alist ${ALIST_SOURCEFILES})
# target_link_libraries (alist ${ALIST_LIBS} )
# 
# chops_install_executables (
#     alist
#   )


if(POLICY CMP0053)
    cmake_policy(SET CMP0053 OLD)
endif()

#declare exe name ############################################################
set(EXENAME "${CHOPS_EXEPREFIX}alist")

#list header files #############################################################

file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include" 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
	list( APPEND AM_${EXENAME}_HEADERFILES "./include/${BASENAME}" )
endforeach( BASENAME )

#list source files #############################################################

file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
	list( APPEND AM_${EXENAME}_SOURCEFILES "${BASENAME}" )
endforeach( BASENAME )

#declare relative paths to internal headers/libs ###############################
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")
set(DFIO_RELPATH "../../dfio")

list(APPEND RELPATH_LIST 
    ${MK4UTIL_RELPATH} 
    ${AFIO_RELPATH} 
    ${DFIO_RELPATH} 
)

#include internal library headers
foreach(path ${RELPATH_LIST})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include exe headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# set library lists ############################################################
set(INTERNAL_LIBLIST 
    "mk4util${CHOPS_LIBPOSTFIX}"
    "afio${CHOPS_LIBPOSTFIX}"
    "dfio${CHOPS_LIBPOSTFIX}"
)

#math library
set(EXTERNAL_LIBLIST m )

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

chops_install_headers (${${EXENAME}_HEADERFILES})
chops_install_executables(${EXENAME})

#configure/generate Makefile.am for autotools build ############################

chops_configure_exe_automake("${RELPATH_LIST}" "${INTERNAL_LIBLIST}" ${EXENAME} "${AM_${EXENAME}_HEADERFILES}" "${AM_${EXENAME}_SOURCEFILES}")
