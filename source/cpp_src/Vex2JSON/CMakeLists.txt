set (VEX2JSON_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexDefinitions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexLine.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexTokenProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexBlockParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VexElementLineGenerator.hh
)

##################################################

set (VEX2JSON_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexDefinitions.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexTokenProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexBlockParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexGenerator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VexElementLineGenerator.cc
)

#declare header paths ##########################################################
include_directories(${EXTERN_HLIBS}/json/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property( SOURCE ${VEX2JSON_SOURCEFILES}
    APPEND PROPERTY COMPILE_DEFINITIONS VEX_FORMAT_DIR="${DATA_INSTALL_DIR}"
)

add_library (MHO_Vex2JSON SHARED ${VEX2JSON_SOURCEFILES} ${VEX2JSON_HEADERFILES})

target_link_libraries (MHO_Vex2JSON MHO_Utilities)

hops_install_headers (${VEX2JSON_HEADERFILES})
hops_install_libraries (MHO_Vex2JSON)

#install vex format specification
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/format)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
