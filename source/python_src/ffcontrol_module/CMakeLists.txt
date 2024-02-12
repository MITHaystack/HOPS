# file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

#declare library name ##########################################################
set(LIBNAME "ffcontrolpy")

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
set(MK4UTIL_RELPATH "../../c_src/mk4util")
set(MSG_RELPATH "../../c_src/msg")
set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol")

list(APPEND RELPATH_LIST ${MK4UTIL_RELPATH} ${MSG_RELPATH} ${FFCONTROL_RELPATH})

#include library headers
foreach(path ${RELPATH_LIST})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#include our own headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#build/link/install library ####################################################
list(APPEND LIBLIST "mk4util" "msg" "ffcontrol")

add_library (${LIBNAME} SHARED ${${LIBNAME}_SOURCEFILES} ${${LIBNAME}_HEADERFILES})

target_link_libraries (${LIBNAME} ${LIBLIST})

hops_install_headers (${${LIBNAME}_HEADERFILES})

hops_install_libraries (${LIBNAME})


#python package setup ##########################################################
if (PY_EXE)

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    #need this target to refresh our copy if anything in the source has changed
    add_custom_target( ffcontrol_copy_src ALL
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    )

    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    #add a build target to catch problems before install (output not used)
    add_custom_target(ffcontrol_build_target ALL 
        COMMAND ${PY_EXE} ${SETUP_PY} build
        DEPENDS hops4py_copy_src ffcontrol_copy_src
    )

    if("${PYTHON_SETUP}" STREQUAL "setuptools")
        #use pip/setuptools
        install(CODE "execute_process(COMMAND ${PY_EXE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} --upgrade --target=${PYTHON_MODULE_INSTALL_DIR} )")
    else()
        #use distutils
        install(CODE "execute_process( COMMAND ${PY_EXE} ${SETUP_PY} install --prefix=${CMAKE_INSTALL_PREFIX} )")
    endif()
    
endif(PY_EXE)
