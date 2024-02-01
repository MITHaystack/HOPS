#list header files #############################################################

set (MK4INTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4CorelInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4VexInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4StationInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4FringeExport.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_LegacyDateConverter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type200Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type201Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type202Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type203Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type204Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type205Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type206Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type207Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type208Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type210Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4Type212Converter.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MK4JSONDateConverter.hh
)

#list source files #############################################################

set( MK4INTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4CorelInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4VexInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4StationInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4FringeExport.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_LegacyDateConverter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type200Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type201Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type202Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type203Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type204Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type205Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type206Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type207Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type208Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type210Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4Type212Converter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MK4JSONDateConverter.cc
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

set(MK4INTERFACE_LIBS mk4util dfio vex MHO_Vex2JSON MHO_Message MHO_Utilities MHO_Containers date-tz)

add_library (MHO_MK4Interface SHARED ${MK4INTERFACE_SOURCEFILES})
target_link_libraries (MHO_MK4Interface ${MK4INTERFACE_LIBS})

hops_install_headers (${MK4INTERFACE_HEADERFILES})
hops_install_libraries (MHO_MK4Interface)

#install aux files
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aux)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
