#add an option on whether or not to use FFTW3
option (HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms" OFF)
if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
endif (HOPS_USE_FFTW3)

#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NDArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryNDArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_CompoundReductions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Reducer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FunctorBroadcaster.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_OperatorStub.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalPaddedFastFourierTransform.hh
)

if(HOPS_USE_FFTW3)
    list(APPEND OPERATOR_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FFTWTypes.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalFastFourierTransformFFTW.hh
    )
endif(HOPS_USE_FFTW3)

#list source files #############################################################

set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_OperatorStub.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(OPERATOR_LIBS MHO_Message MHO_Math)

if(HOPS_USE_FFTW3)
    list(APPEND OPERATOR_LIBS ${FFTW_LIBRARIES} ${FFTW_SINGLE_LIBRARIES} ${FFTW_LONG_LIBRARIES})
endif(HOPS_USE_FFTW3)

add_library (MHO_Operators SHARED ${OPERATOR_SOURCEFILES})
target_link_libraries (MHO_Operators ${OPERATOR_LIBS})

hops_install_headers (${OPERATOR_HEADERFILES})
hops_install_libraries (MHO_Operators)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
