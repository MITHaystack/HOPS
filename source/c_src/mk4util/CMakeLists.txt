#declare library name ##########################################################
set(LIBNAME "mk4util")

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
set(MSG_RELPATH "../msg")
list(APPEND RELPATH_LIST ${MSG_RELPATH})

#include library headers
foreach(path ${RELPATH_LIST})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

set(INC_RELPATH "include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${INC_RELPATH})

#NOTE: the 'text' (share/text) directory is resolved at runtime in environment.c
#(via dladdr on libmk4util.so) so that no absolute install path is baked into the
#library. The flag ${CMAKE_DL_LIBS} (added to the link below) provides
#dladdr, which is needed to figure out the install prefix at runtime.

if(HOPS3_USE_CXX)
SET_SOURCE_FILES_PROPERTIES(${${LIBNAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
endif(HOPS3_USE_CXX)

#build/link/install library ####################################################
list(APPEND LIBLIST "msg")

#math library (+ ${CMAKE_DL_LIBS} for dladdr used by textdef() in environment.c)
set(EXTERNAL_LIBLIST m ${CMAKE_DL_LIBS})

add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

target_link_libraries (${LIBNAME} ${LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${LIBNAME}_HEADERFILES})
hops_install_libraries (${LIBNAME})
legacy_hops_install_headers (${${LIBNAME}_HEADERFILES})
