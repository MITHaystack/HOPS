#buid a temporary wrapper class around the fourfit ffcontrol c-library 
#so we can easily build control blocks from old-style control files
#this links against old fourfit libraries (temporary measure while we transition)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(FFCONTROLINTER_LIBS MHO_Message MHO_Utilities)

#list header files 
list(APPEND FFCONTROLINTER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FourfitControlInterface.hh
)

#list source files
list(APPEND FFCONTROLINTER_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FourfitControlInterface.cc
)

#declare header paths for old c-libs ##########################################
set(MK4UTIL_RELPATH "../../../c_src/mk4util")
set(DFIO_RELPATH "../../../c_src/dfio")
set(VEX_RELPATH "../../../c_src/vex")
set(VEX_RELPATH "../../../c_src/msg")
set(FFCONTROL_RELPATH "../../../c_src/fourfit_libs/ffcontrol")

list(APPEND RELPATH_LIST
    ${MK4UTIL_RELPATH} ${DFIO_RELPATH} ${VEX_RELPATH} ${FFCONTROL_RELPATH}
)

#include library headers
foreach(path ${RELPATH_LIST})
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
endforeach(path)

#add old c libs ################################################################
list(APPEND FFCONTROLINTER_LIBS "mk4util" "dfio" "vex" "ffcontrol" )

#compile and install library ###################################################
add_library (MHO_ffcontrol_interface SHARED ${FFCONTROLINTER_SOURCEFILES})
target_link_libraries (MHO_ffcontrol_interface ${FFCONTROLINTER_LIBS})

hops_install_headers (${FFCONTROLINTER_HEADERFILES})
hops_install_libraries (MHO_ffcontrol_interface )
