#declare exe name ############################################################
set(EXENAME "alist")

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
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
	list( APPEND AM_${EXENAME}_SOURCEFILES "${BASENAME}" )
endforeach( BASENAME )

list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/alist.c)
list(INSERT AM_${EXENAME}_SOURCEFILES 0 alist.c)


# file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
#   "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
# 
# file(GLOB ${EXENAME}_EXTRA_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src"
#   "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
# 
# foreach( BASENAME ${${EXENAME}_EXTRA_SOURCE_BASENAMES} )
# 	list( APPEND ${EXENAME}_SOURCE_BASENAMES "${BASENAME}" )
# endforeach( BASENAME )
# 
# set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
# 
# foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
# 	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
# 	list( APPEND AM_${EXENAME}_SOURCEFILES "${BASENAME}" )
# endforeach( BASENAME )

#declare relative paths to internal headers/libs ###############################
set(MSG_RELPATH "../../msg")
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")
set(DFIO_RELPATH "../../dfio")

list(APPEND RELPATH_LIST
    ${MSG_RELPATH}
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
    "msg"
    "mk4util"
    "afio"
    "dfio"
)

#math library
set(EXTERNAL_LIBLIST m)

if(HOPS3_USE_CXX)
    SET_SOURCE_FILES_PROPERTIES(${${EXENAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
endif(HOPS3_USE_CXX)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
