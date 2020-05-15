# pre-requisites ###############################################################
find_package (FFTW REQUIRED)

#declare library name ##########################################################
set(LIBNAME "ffsearch${CHOPS_LIBPOSTFIX}")

#list header files #############################################################

file(GLOB ${LIBNAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include" 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${LIBNAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${LIBNAME}_HEADER_BASENAMES} )
	list( APPEND ${LIBNAME}_HEADERFILES ${${LIBNAME}_HEADER_PATH}/${BASENAME} )
	list( APPEND AM_${LIBNAME}_HEADERFILES "./include/${BASENAME}" )
endforeach( BASENAME )

#list source files #############################################################

file(GLOB ${LIBNAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set( ${LIBNAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

foreach( BASENAME ${${LIBNAME}_SOURCE_BASENAMES} )
	list( APPEND ${LIBNAME}_SOURCEFILES ${${LIBNAME}_SOURCE_PATH}/${BASENAME} )
	list( APPEND AM_${LIBNAME}_SOURCEFILES "./src/${BASENAME}" )
endforeach( BASENAME )

#declare header paths ##########################################################
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")
set(DFIO_RELPATH "../../dfio")
set(VEX_RELPATH "../../vex")
set(FFCONTROL_RELPATH "../ffcontrol")
set(FFCORE_RELPATH "../ffcore")
set(FFMATH_RELPATH "../ffmath")
set(FFIO_RELPATH "../ffio")

list(APPEND RELPATH_LIST ${MK4UTIL_RELPATH} ${AFIO_RELPATH} ${DFIO_RELPATH} ${VEX_RELPATH} ${FFCONTROL_RELPATH} ${FFCORE_RELPATH} ${FFIO_RELPATH} ${FFMATH_RELPATH})

#include library headers
foreach(path ${RELPATH_LIST})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include our own headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#external includes
include_directories(${FFTW_INCLUDE_DIR})

#build/link/install library ####################################################
list(APPEND LIBLIST "mk4util${CHOPS_LIBPOSTFIX}" "afio${CHOPS_LIBPOSTFIX}" 
                    "dfio${CHOPS_LIBPOSTFIX}" "vex${CHOPS_LIBPOSTFIX}" 
                    "ffcontrol${CHOPS_LIBPOSTFIX}" "ffcore${CHOPS_LIBPOSTFIX}" 
                    "ffio${CHOPS_LIBPOSTFIX}" "ffmath${CHOPS_LIBPOSTFIX}" 
                    "${FFTW_LIBRARIES}"
    )

add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

target_link_libraries (${LIBNAME} ${LIBLIST})

chops_install_headers (${${LIBNAME}_HEADERFILES})
chops_install_libraries (${LIBNAME})

#configure/generate Makefile.am for autotools build ############################

chops_configure_automake("${RELPATH_LIST}" "${LIBLIST}" ${LIBNAME} "${AM_${LIBNAME}_HEADERFILES}" "${AM_${LIBNAME}_SOURCEFILES}")
