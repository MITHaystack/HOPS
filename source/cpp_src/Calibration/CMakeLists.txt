#list header files #############################################################
set (CALIBRATION_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelLabeller.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_WeightChannelizer.hh
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelPhaseCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NormFX.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DelayRate.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MBDelaySearch.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InterpolateFringePeak.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ComputePlotData.hh
)

#list source files #############################################################
set( CALIBRATION_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VisibilityChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_WeightChannelizer.cc
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelPhaseCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NormFX.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DelayRate.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MBDelaySearch.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_InterpolateFringePeak.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ComputePlotData.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(CALIBRATION_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers)

#compile and install library ###################################################
add_library (MHO_Calibration SHARED ${CALIBRATION_SOURCEFILES})
target_link_libraries (MHO_Calibration ${CALIBRATION_LIBS})

hops_install_headers (${CALIBRATION_HEADERFILES})
hops_install_libraries (MHO_Calibration )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
