#buid a temporary wrapper around the fourfit ffcontrol c-library 
#so we can easily build control blocks from old-style control files
#this links against old fourfit libraries (temporary measure while we transition)

#list header files #############################################################
set (CONTROL_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FourfitControlInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlBlockWrapper.hh
)

#list source files #############################################################
set( CONTROL_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FourfitControlInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlBlockWrapper.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(CONTROL_LIBS MHO_Message MHO_Utilities MHO_Containers mk4util msg ffcontrol)

#declare header paths for old c-libs ##########################################
set(MK4UTIL_RELPATH "../../c_src/mk4util")
set(MSG_RELPATH "../../c_src/msg")
set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol")

list(APPEND RELPATH_LIST ${MK4UTIL_RELPATH} ${MSG_RELPATH} ${FFCONTROL_RELPATH}
)

#include library headers
foreach(path ${RELPATH_LIST})
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#compile and install library ###################################################
add_library (MHO_Control SHARED ${CONTROL_SOURCEFILES})
target_link_libraries (MHO_Control ${CONTROL_LIBS})

hops_install_headers (${CONTROL_HEADERFILES})
hops_install_libraries (MHO_Control )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
