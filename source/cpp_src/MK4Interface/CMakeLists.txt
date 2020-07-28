#list header files #############################################################

set (MK4INTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkMK4Interface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/HkMK4CorelInterface.hh
)

#list source files #############################################################

set( MK4INTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkMK4Interface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HkMK4CorelInterface.cc
)

#declare header paths ##########################################################
set(MK4STUB_RELPATH "../../c_src/mk4stub/include")
set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(DFIO_RELPATH "../../c_src/dfio/include")
set(VEX_RELPATH "../../c_src/vex/include")
set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol/include")
set(FFCORE_RELPATH"../../c_src/fourfit_libs/ffcore/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4STUB_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${FFCONTROL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${FFCORE_RELPATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(MK4INTERFACELIBS mk4stub mk4util dfio vex ffcontrol ffcore)

add_library (HMK4Interface SHARED ${MK4INTERFACE_SOURCEFILES})
target_link_libraries (HMK4Interface ${MK4INTERFACE_LIBS})

hops_install_headers (${MK4INTERFACE_HEADERFILES})
hops_install_libraries (HMK4Interface)
