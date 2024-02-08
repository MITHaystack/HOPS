#list header files #############################################################
set (INITIALIZATION_HEADERFILES
    #parameters
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ParameterConfigurator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ParameterManager.hh
    #labeling operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelLabelerBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SamplerLabelerBuilder.hh
    #selection operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DataSelectionBuilder.hh
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelQuantity.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelPhaseCorrectionBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelDelayCorrectionBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualPolPhaseCorrectionBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualPolDelayCorrectionBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultitonePhaseCorrectionBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PolProductSummationBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_LinearDParCorrectionBuilder.hh
    #manager
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_OperatorBuilderManager.hh
)

#list source files #############################################################
set( INITIALIZATION_SOURCEFILES
    #parameters
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ParameterConfigurator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ParameterManager.cc
    #labeling operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelLabelerBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SamplerLabelerBuilder.cc
    #selection operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DataSelectionBuilder.cc
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelQuantity.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelPhaseCorrectionBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelDelayCorrectionBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualPolPhaseCorrectionBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualPolDelayCorrectionBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MultitonePhaseCorrectionBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PolProductSummationBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_LinearDParCorrectionBuilder.cc
    #manager
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_OperatorBuilderManager.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(INITIALIZATION_LIBS MHO_Message MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration)

#compile and install library ###################################################
add_library (MHO_Initialization SHARED ${INITIALIZATION_SOURCEFILES})
target_link_libraries (MHO_Initialization ${INITIALIZATION_LIBS})

hops_install_headers (${INITIALIZATION_HEADERFILES})
hops_install_libraries (MHO_Initialization )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
