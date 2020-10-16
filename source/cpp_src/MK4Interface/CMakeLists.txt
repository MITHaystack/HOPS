#list header files #############################################################

set (MK4INTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMK4Interface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMK4Visibility.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMK4CorelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMK4VexInterface.hh
)

#list source files #############################################################

set( MK4INTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOMK4Interface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOMK4CorelInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOMK4VexInterface.cc
)

#declare header paths ##########################################################
set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(DFIO_RELPATH "../../c_src/dfio/include")
set(VEX_RELPATH "../../c_src/vex/include")
set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol/include")
set(FFCORE_RELPATH"../../c_src/fourfit_libs/ffcore/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${FFCONTROL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${FFCORE_RELPATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(MK4INTERFACE_LIBS mk4util dfio vex ffcontrol ffcore)

add_library (MHOMK4Interface SHARED ${MK4INTERFACE_SOURCEFILES})
target_link_libraries (MHOMK4Interface ${MK4INTERFACE_LIBS})

hops_install_headers (${MK4INTERFACE_HEADERFILES})
hops_install_libraries (MHOMK4Interface)
