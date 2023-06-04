#list header files #############################################################
set (BUILDERS_HEADERFILES
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelLabellerBuilder.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelPhaseCorrectionBuilder.hh

)

#list source files #############################################################
set( BUILDERS_SOURCEFILES
    #calibration operators
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelLabellerBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelPhaseCorrectionBuilder.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(BUILDERS_LIBS MHO_Message MHO_Utilities MHO_Operators MHO_Containers MHO_Control MHO_Calibration)

#compile and install library ###################################################
add_library (MHO_Builders SHARED ${BUILDERS_SOURCEFILES})
target_link_libraries (MHO_Builders ${BUILDERS_LIBS})

hops_install_headers (${BUILDERS_HEADERFILES})
hops_install_libraries (MHO_Builders )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
