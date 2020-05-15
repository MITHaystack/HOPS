#declare library name ##########################################################
set(LIBNAME "mk4util${CHOPS_LIBPOSTFIX}")

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

set(INC_RELPATH "include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${INC_RELPATH})

#build/link/install library ####################################################

add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

target_link_libraries (${LIBNAME})

chops_install_headers (${${LIBNAME}_HEADERFILES})
chops_install_libraries (${LIBNAME})

#configure/generate Makefile.am for autotools build ############################

string (REPLACE ";" " " HEADER_BASENAMES_STR "${AM_${LIBNAME}_HEADERFILES}")
string (REPLACE ";" " " SOURCE_BASENAMES_STR "${AM_${LIBNAME}_SOURCEFILES}")

set(MAKEFILE_AM_IN "${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am.in")
set(MAKEFILE_AM    "${CMAKE_CURRENT_SOURCE_DIR}/Makefile.am")

configure_file(${MAKEFILE_AM_IN} ${MAKEFILE_AM} @ONLY)
