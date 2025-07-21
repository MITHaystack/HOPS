#list header files #############################################################

set (MK4INTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4CorelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4VexInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4StationInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4FringeExport.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4ScanConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4CorelInterfaceReversed.hh
    #${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4StationInterfaceReversed.hh
    #${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4ScanConverterReversed.hh
)

#list source files #############################################################

set( MK4INTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4CorelInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4VexInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4StationInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4FringeExport.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4ScanConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4CorelInterfaceReversed.cc
    #${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4StationInterfaceReversed.cc
    #${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4ScanConverterReversed.cc
)

#declare header paths ##########################################################
set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(DFIO_RELPATH "../../c_src/dfio/include")
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

include_directories(${EXTERN_HLIBS}/date/include) #date library

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property( SOURCE ${MK4INTERFACE_SOURCEFILES}
    APPEND PROPERTY COMPILE_DEFINITIONS HOPS_MK4AUX_DIR="${DATA_INSTALL_DIR}"
)

#compile and install library ###################################################

set(MK4INTERFACE_LIBS mk4util dfio vex MHO_Vex2JSON MHO_Message MHO_Utilities MHO_Containers MHO_Operators MHO_Calibration date-tz)

add_library (MHO_MK4Interface SHARED ${MK4INTERFACE_SOURCEFILES})
target_link_libraries (MHO_MK4Interface ${MK4INTERFACE_LIBS})

hops_install_headers (${MK4INTERFACE_HEADERFILES})
hops_install_libraries (MHO_MK4Interface)

#install aux files
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aux)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
