set (VEX2JSON_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JSONHeaderWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JSONOutputObject.hh
    #mk4_typedefs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DateStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SkyCoordStructWrapper.hh
    # #ovex
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SourceStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChanStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScanStructWrapper.hh
)

##################################################

set (VEX2JSON_SOURCEFILES
    #mk4_typedefs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DateStructWrapper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SkyCoordStructWrapper.cc
    # #ovex
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SourceStructWrapper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChanStructWrapper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_StationStructWrapper.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ScanStructWrapper.cc
)

#declare header paths ##########################################################
set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})
include_directories(${EXTERN_HLIBS}/json/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library (vex2json SHARED ${VEX2JSON_SOURCEFILES} ${VEX2JSON_HEADERFILES})

target_link_libraries (vex2json vex mk4util)

hops_install_headers (${VEX2JSON_HEADERFILES})
hops_install_libraries (vex2json)


if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
