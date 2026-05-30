#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

#list of c++ libraries
set(TEST_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers MHO_Calibration)

#compile and install test executables
set(SOURCE_BASENAMES
    TestChannelLabeler
    TestSamplerLabeler
    #TestReadAdhocFlagFile
)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestMinWeight
        TestSBDTableGenerator
        TestMHO_DelayRate
        TestMHO_DelayModel
        TestMHO_MBDelaySearch
        TestMHO_StationModel
        TestInterpolateFringePeak
        TestInterpolateFringePeakOptimized
        TestInterpolateSpectralLinePeak
        TestDoubleSidebandChannelLabeler
        TestPolarizationRelabeler
        TestPolarizationProductRelabeler
        TestAdhocFlagging
        TestAdhocPhaseCorrection
        TestCircularFieldRotationCorrection
        TestDCBlock
        TestIonosphericPhaseCorrection
        TestLinearDParCorrection
        TestLSBOffset
        TestManualChannelDelayCorrection
        TestManualChannelPhaseCorrection
        TestManualPolDelayCorrection
        TestManualPolPhaseCorrection
        TestMixedPolYShift
        TestMixedSidebandNormFX
        TestMultitonePhaseCorrection
        TestNormFX
        TestNotchComb
        TestNotches
        TestPassband
        TestPhaseCalibrationTrim
        TestStationDelayCorrection
        TestPolProductSummation
        TestSingleSidebandNormFX
        TestVisibilityChannelizer
        TestWeightChannelizer
        TestEstimatePCManual
    )
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )
