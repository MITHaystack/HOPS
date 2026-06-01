set (CONTROL_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlDefinitions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlFileParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlElementParser.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlTokenProcessor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ControlConditionEvaluator.hh
)

##################################################

set (CONTROL_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlDefinitions.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlFileParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlElementParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlTokenProcessor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ControlConditionEvaluator.cc
)

#declare header paths ##########################################################
include_directories(${EXTERN_HLIBS}/json/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#NOTE: the control-format directory is resolved at runtime (see MHO_DirectoryInterface::GetHopsInstallPrefix)
#so that no absolute install path is baked into the binary.

add_library (MHO_Control SHARED ${CONTROL_SOURCEFILES} ${CONTROL_HEADERFILES})

target_link_libraries (MHO_Control MHO_Utilities MHO_Message)

hops_install_headers (${CONTROL_HEADERFILES})
hops_install_libraries (MHO_Control)

#install hops control format specification
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/format)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
