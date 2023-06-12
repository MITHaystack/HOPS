#list header files #############################################################
set (INITIALIZATION_HEADERFILES
    #labelling operators 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelLabellerBuilder.hh
    #selection operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DataSelectionBuilder.hh
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelQuantity.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelPhaseCorrectionBuilder.hh
    #manager
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_OperatorBuilderManager.hh
)

#list source files #############################################################
set( INITIALIZATION_SOURCEFILES
    #labelling operators 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelLabellerBuilder.cc
    #selection operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DataSelectionBuilder.cc
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelQuantity.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelPhaseCorrectionBuilder.cc
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
