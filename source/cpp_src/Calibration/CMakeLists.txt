#list header files #############################################################
set (CALIBRATION_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelLabeler.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SamplerLabeler.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_WeightChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DoubleSidebandChannelLabeler.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelPhaseCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualChannelDelayCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualPolPhaseCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ManualPolDelayCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultitonePhaseCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PolProductSummation.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_LinearDParCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_CircularFieldRotationCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IonosphericPhaseCorrection.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_EstimatePCManual.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Passband.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Notches.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DCBlock.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MinWeight.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SingleSidebandNormFX.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NormFX.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SBDTableGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DelayRate.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MBDelaySearch.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InterpolateFringePeak.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DelayModel.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_StationModel.hh
)

#list source files #############################################################
set( CALIBRATION_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VisibilityChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_WeightChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelPhaseCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualChannelDelayCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualPolPhaseCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ManualPolDelayCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MultitonePhaseCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PolProductSummation.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_LinearDParCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_CircularFieldRotationCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IonosphericPhaseCorrection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_EstimatePCManual.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Passband.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Notches.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DCBlock.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MinWeight.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SingleSidebandNormFX.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NormFX.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SBDTableGenerator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DelayRate.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MBDelaySearch.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_InterpolateFringePeak.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DelayModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_StationModel.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(CALIBRATION_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers)

#compile and install library ###################################################
add_library (MHO_Calibration SHARED ${CALIBRATION_SOURCEFILES})
target_link_libraries (MHO_Calibration ${CALIBRATION_LIBS})

hops_install_headers (${CALIBRATION_HEADERFILES})
hops_install_libraries (MHO_Calibration )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
