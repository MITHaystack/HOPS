find_package (PGPLOT)
find_package (X11)
find_package (GFORTRAN)

if( PGPLOT_FOUND AND X11_FOUND AND GFORTRAN_FOUND )
    
#declare exe name ############################################################
set(EXENAME "search")
set(EXENAME2 "soirch")

#list header files #############################################################

file(GLOB ${EXENAME}_HEADER_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

set( ${EXENAME}_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach( BASENAME ${${EXENAME}_HEADER_BASENAMES} )
	list( APPEND ${EXENAME}_HEADERFILES ${${EXENAME}_HEADER_PATH}/${BASENAME} )
endforeach( BASENAME )

#list source files #############################################################

file(GLOB ${EXENAME}_SOURCE_BASENAMES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set( ${EXENAME}_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

foreach( BASENAME ${${EXENAME}_SOURCE_BASENAMES} )
	list( APPEND ${EXENAME}_SOURCEFILES ${${EXENAME}_SOURCE_PATH}/${BASENAME} )
endforeach( BASENAME )

list(INSERT ${EXENAME}_SOURCEFILES 0 ${CMAKE_CURRENT_SOURCE_DIR}/search.c)


#declare relative paths to internal headers/libs ###############################
set(MSG_RELPATH "../../msg")
set(MK4UTIL_RELPATH "../../mk4util")
set(AFIO_RELPATH "../../afio")
set(DFIO_RELPATH "../../dfio")
set(VEX_RELPATH "../../vex")

list(APPEND RELPATH_LIST
    ${MSG_RELPATH}
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

# set library lists ############################################################
set(INTERNAL_LIBLIST
    "msg"
    "mk4util"
    "afio"
    "dfio"
    "vex"
)


#include external library headers
include_directories(${PGPLOT_INCLUDE_DIR})
include_directories(${FFTW_INCLUDE_DIR})

#TODO...determine why CPGPLOT and PGPLOT must be given in opposite order from fourfit?

set(EXTERNAL_LIBLIST
    ${CPGPLOT_LIBRARY}
    ${PGPLOT_LIBRARY}
)

#in case the user has compiled PGPLOT against libPNG, we link against it here
find_package(PNG)
if(PNG_FOUND)
list(APPEND EXTERNAL_LIBLIST ${PNG_LIBRARIES})
endif(PNG_FOUND)

list(APPEND EXTERNAL_LIBLIST
    ${GFORTRAN_LIBRARIES}
    ${X11_LIBRARIES}
    "m"
)

if(HOPS3_USE_CXX)
    SET_SOURCE_FILES_PROPERTIES(${${EXENAME}_SOURCEFILES} PROPERTIES LANGUAGE CXX )
endif(HOPS3_USE_CXX)

add_executable (${EXENAME} ${${EXENAME}_SOURCEFILES} )
add_executable (${EXENAME2} ${${EXENAME}_SOURCEFILES} )

set_target_properties( ${EXENAME} PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DBIGGER=1")
set_target_properties( ${EXENAME2} PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DBIGGER=0")

target_link_libraries (${EXENAME} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})
target_link_libraries (${EXENAME2} ${INTERNAL_LIBLIST} ${EXTERNAL_LIBLIST})

hops_install_headers (${${EXENAME}_HEADERFILES})

hops_install_executables(${EXENAME})
hops_install_executables(${EXENAME2})

else()
    
    if( NOT PGPLOT_FOUND )
        message(WARNING "PGPLOT library was not found, will not build and install search.")
    endif()
    
    if( NOT X11_FOUND )
            message(WARNING "X11 library was not found, will not build and install search.")
    endif()
    
    if( NOT GFORTRAN_FOUND )
            message(WARNING "gfortran compiler/library was not found, will not build and install search.")
    endif()

endif()
