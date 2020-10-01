# pre-requisites ###############################################################
find_package (PGPLOT REQUIRED)
find_package (FFTW REQUIRED)
find_package (X11 REQUIRED)
find_package (GFORTRAN REQUIRED)

#declare exe name ############################################################
set(EXENAME "aedit")

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

list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/aedit.c)
list(INSERT AM_${EXENAME}_SOURCEFILES 0 aedit.c)

#declare relative paths to internal headers/libs ###############################
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")
set(DFIO_RELPATH "../../dfio")
set(VEX_RELPATH "../../vex")


list(APPEND RELPATH_LIST
    ${MK4UTIL_RELPATH}
    ${AFIO_RELPATH}
    ${DFIO_RELPATH}
    ${VEX_RELPATH}
)

#include internal library headers
foreach(path ${RELPATH_LIST})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include exe headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#include external library headers
include_directories(${PGPLOT_INCLUDE_DIR})
include_directories(${FFTW_INCLUDE_DIR})

# set library lists ############################################################
set(INTERNAL_LIBLIST
    "mk4util"
    "afio"
    "dfio"
    "vex"
)

set(EXTERNAL_LIBLIST
    ${FFTW_LIBRARIES}
    ${PGPLOT_LIBRARIES}
    ${X11_LIBRARIES}
    ${GFORTRAN_LIBRARIES}
    m
)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
