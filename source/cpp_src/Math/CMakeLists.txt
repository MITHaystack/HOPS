#list header files #############################################################
set (MATH_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BitReversalPermutation.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FastFourierTransformUtilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FastFourierTransformWorkspace.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_CheckForNaN.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SimulatedSignalGenerator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_GaussianWhiteNoiseSignal.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SingleToneSignal.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_UniformGridPointsCalculator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringeRotation.hh
)


#list source files #############################################################
set( MATH_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BitReversalPermutation.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_GaussianWhiteNoiseSignal.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SingleToneSignal.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_UniformGridPointsCalculator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringeRotation.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(MATH_LIBS MHO_Message)

add_library (MHO_Math SHARED ${MATH_SOURCEFILES})
target_link_libraries (MHO_Math ${MATH_LIBS})

hops_install_headers (${MATH_HEADERFILES})
hops_install_libraries (MHO_Math )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
