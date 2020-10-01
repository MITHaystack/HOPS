if(POLICY CMP0053)
    cmake_policy(SET CMP0053 OLD)
endif()


# pre-requisites ###############################################################
find_package (PGPLOT REQUIRED)
find_package (FFTW REQUIRED)
find_package (X11 REQUIRED)
find_package (GFORTRAN REQUIRED)

#declare exe name ############################################################
set(EXENAME "${CHOPS_EXEPREFIX}fourfit")

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
set(VEX_RELPATH "../../vex")
set(FFCONTROL_RELPATH "../../fourfit_libs/ffcontrol")
set(FFCORE_RELPATH "../../fourfit_libs/ffcore")
set(FFMATH_RELPATH "../../fourfit_libs/ffmath")
set(FFIO_RELPATH "../../fourfit_libs/ffio")
set(FFSEARCH_RELPATH "../../fourfit_libs/ffsearch")
set(FFPLOT_RELPATH "../../fourfit_libs/ffplot")

list(APPEND RELPATH_LIST
    ${MK4UTIL_RELPATH}
    ${AFIO_RELPATH}
    ${DFIO_RELPATH}
    ${VEX_RELPATH}
    ${FFCONTROL_RELPATH}
    ${FFMATH_RELPATH}
    ${FFCORE_RELPATH}
    ${FFIO_RELPATH}
    ${FFSEARCH_RELPATH}
    ${FFPLOT_RELPATH}
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

#yes...the number of escapes '\' needed here is ridiculous,
#but there is no other easier way to pass these flags to automake
#(this has not been tested on platforms other than bash/linux)
set(FF_PROGNAME_DEF "FF_PROGNAME=\\\\\"fourfit\\\\\"")
set(FF_VER_NO_DEF "FF_VER_NO=\\\\\"${MODULE_VERSION_NUMBER}\\\\\"")

chops_add_cflag(${FF_PROGNAME_DEF})
chops_add_cflag(${FF_VER_NO_DEF})

# set library lists ############################################################
set(INTERNAL_LIBLIST
    "mk4util"
    "afio"
    "dfio"
    "vex"
    "ffcontrol"
    "ffmath"
    "ffcore"
    "ffio"
    "ffsearch"
    "ffplot"
)

set(EXTERNAL_LIBLIST
    ${FFTW_LIBRARIES}
    ${PGPLOT_LIBRARIES}
    ${X11_LIBRARIES}
    ${GFORTRAN_LIBRARIES}
)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${EXENAME}_HEADERFILES})
hops_install_executables(${EXENAME})
