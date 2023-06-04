
#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Operator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_OperatorToolbox.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UnaryOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_InspectingOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_TransformingOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UnaryFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BinaryOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_CompoundReductions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Reducer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FunctorBroadcaster.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NaNMasker.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ComplexConjugator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_AbsoluteValue.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_CyclicRotator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SubSample.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SelectRepack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalPaddedFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ElementTypeCaster.hh
)

if(HOPS_USE_FFTW3)
    list(APPEND OPERATOR_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FFTWTypes.hh
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MultidimensionalFastFourierTransformFFTW.hh
    )
endif(HOPS_USE_FFTW3)

#list source files #############################################################

set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Operator.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(OPERATOR_LIBS MHO_Message MHO_Math MHO_Containers) 

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
