#declare exe name ############################################################
set(EXENAME "CorAsc2")
set(EXENAME2 "CorPrune")

#list header files #############################################################

file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
endforeach( BASENAME )

#list source files #############################################################

set(${EXENAME}_SOURCEFILES "${CMAKE_CURRENT_SOURCE_DIR}/src/CorAsc2.c")
set(${EXENAME2}_SOURCEFILES "${CMAKE_CURRENT_SOURCE_DIR}/src/CorPrune.c")

#declare relative paths to internal headers/libs ###############################
set(MSG_RELPATH "../../../msg")
set(MK4UTIL_RELPATH "../../../mk4util")
set(DFIO_RELPATH "../../../dfio")

list(APPEND RELPATH_LIST
    ${MSG_RELPATH}
    ${MK4UTIL_RELPATH}
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
    "msg"
    "mk4util"
    "dfio"
)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

add_executable (${EXENAME2} ${${EXENAME2}_SOURCEFILES} )
target_link_libraries (${EXENAME2} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
hops_install_executables(${EXENAME2})

