#declare library name ##########################################################
set(LIBNAME "ffmath${CHOPS_LIBPOSTFIX}")

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
set(DFIO_RELPATH "../../dfio")

list(APPEND RELPATH_LIST ${MK4UTIL_RELPATH} ${DFIO_RELPATH} )

#include library headers
foreach(path ${RELPATH_LIST})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include our own headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#build/link/install library ####################################################
list(APPEND LIBLIST "mk4util${CHOPS_LIBPOSTFIX}" "dfio${CHOPS_LIBPOSTFIX}")

add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

target_link_libraries (${LIBNAME} ${LIBLIST})

chops_install_headers (${${LIBNAME}_HEADERFILES})
chops_install_libraries (${LIBNAME})

#configure/generate Makefile.am for autotools build ############################

chops_configure_automake("${RELPATH_LIST}" "${LIBLIST}" ${LIBNAME} "${AM_${LIBNAME}_HEADERFILES}" "${AM_${LIBNAME}_SOURCEFILES}")
