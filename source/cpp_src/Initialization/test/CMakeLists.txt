include_directories(${EXTERN_HLIBS}/json/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS
    MHO_Message MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration MHO_Initialization
)

set(SOURCE_BASENAMES
    TestChannelLabelerBuilder
    TestManualChannelPhaseCorrectionBuilder
    TestChannelUtilities
    TestOperatorBuilderBase
    TestDCBlockBuilder
    TestMinWeightBuilder
    TestLSBOffsetBuilder
    TestMixedPolYShiftBuilder
    TestStationDelayCorrectionBuilder
    TestNotchesBuilder
    TestPassbandBuilder
    TestNotchCombBuilder
    TestManualPolPhaseCorrectionBuilder
    TestManualPolDelayCorrectionBuilder
    TestManualChannelDelayCorrectionBuilder
    TestLinearDParCorrectionBuilder
    TestPolarizationRelabelerBuilder
    TestPolProductSummationBuilder
    TestCircularFieldRotationBuilder
    TestAdhocFlaggingBuilder
    TestSamplerLabelerBuilder
    TestMultitonePhaseCorrectionBuilder
    TestAdhocPhaseCorrectionBuilder
    TestParameterConfigurator
    TestParameterManager
    TestOperatorBuilderManager
    TestDataSelectionBuilder
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )
