#declare header paths ##########################################################
hops_internal_include_directories()

set (HOPS_TEST_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Containers MHO_Operators)

set(SOURCE_BASENAMES
    TestMultidimensionalFastFourierTransform
    TestMultidimensionalFastFourierTransform2
    TestMultidimensionalFastFourierTransform3
    TestMultidimensionalPaddedFastFourierTransform
    #TestInterpolatedFFT
    TestEndZeroPadder
    TestReducer
    TestPointwiseAdd
    TestPointwiseMultiply
    TestCyclicRotation
    TestSubSample
)

#for testing purposes only -- ROOT not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_internal_include_directories(../../Plugins/Root/include)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND HOPS_TEST_LIBS MHO_RootPlugin ${ROOT_LIBRARIES})
endif()


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

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
