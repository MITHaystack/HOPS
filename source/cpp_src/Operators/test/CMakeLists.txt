#declare header paths ##########################################################
hops_internal_include_directories()

set (HOPS_TEST_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Containers MHO_Operators)

set(SOURCE_BASENAMES
    TestMultidimensionalFastFourierTransform
    TestMultidimensionalFastFourierTransform2
    TestMultidimensionalFastFourierTransform3
    #TestInterpolatedFFT
    TestEndZeroPadder
    TestReducer
    TestPointwiseAdd
    TestPointwiseMultiply
    TestCyclicRotation
    TestSubSample
)

if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND HOPS_TEST_LIBS ${FFTW_LIBRARIES})
    list(APPEND SOURCE_BASENAMES
        TestMultidimensionalFastFourierTransformFFTW
        TestMultidimensionalFastFourierTransformFFTW2
        TestMultidimensionalFastFourierTransformFFTW3
    )
endif (HOPS_USE_FFTW3)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestMHO_AbsoluteValue
        TestMHO_EndZeroPadder
        TestMHO_EndZeroPadderOptimized
        TestMHO_Operator
        TestMHO_OperatorToolbox
        TestMHO_CyclicRotator
        TestMHO_Reducer
        TestMHO_UnaryOperator
        TestMHO_UnaryInPlaceOperator
        TestMHO_BinaryOperator
        TestMHO_TransformingOperator
        TestMHO_FastFourierTransform
        TestMHO_MultidimensionalFastFourierTransform
        TestMHO_FunctorBroadcaster
        TestMHO_ComplexConjugator
        TestMHO_ElementTypeCaster
        TestMHO_ExtremaSearch
        TestMHO_NaNMasker
        TestMHO_SelectRepack
        TestMHO_SubSample
    )
endif()

if(HOPS_USE_FFTW3 AND HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestMHO_MultidimensionalFastFourierTransformFFTW
    )
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )
