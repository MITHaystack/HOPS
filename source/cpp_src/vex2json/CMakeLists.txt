set (VEX2JSON_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JSONOutputObject.hh
    #mk4_typedefs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DateStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SkyCoordStructWrapper.hh
    # #ovex
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SourceStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChanStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationStructWrapper.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ScanStructWrapper.hh
    #new stuff
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexLine.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexTokenProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexBlockParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexGenerator.hh
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
    #new stuff 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexTokenProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexBlockParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexGenerator.cc
)

#declare header paths ##########################################################
set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})
include_directories(${EXTERN_HLIBS}/json/include)

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


set_property( SOURCE ${VEX2JSON_SOURCEFILES}
    APPEND PROPERTY COMPILE_DEFINITIONS VEX_FORMAT_DIR="${DATA_INSTALL_DIR}"
)

add_library (vex2json SHARED ${VEX2JSON_SOURCEFILES} ${VEX2JSON_HEADERFILES})

target_link_libraries (vex2json vex mk4util MHO_Utilities)

hops_install_headers (${VEX2JSON_HEADERFILES})
hops_install_libraries (vex2json)

#install vex format specification
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/format)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
