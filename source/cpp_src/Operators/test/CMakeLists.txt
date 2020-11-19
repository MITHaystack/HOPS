#declare header paths ##########################################################
hops_internal_include_directories()

set (HOPS_TEST_LIBS MHOMessage MHOMath MHOUtilities MHOOperators)

set(SOURCE_BASENAMES
    TestMultidimensionalFastFourierTransform
    TestArrayReducer
)

if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND HOPS_TEST_LIBS ${FFTW_LIBRARIES})
endif (HOPS_USE_FFTW3)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
