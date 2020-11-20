#add an option on whether or not to use FFTW3
option (HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms" OFF)
if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
endif (HOPS_USE_FFTW3)

#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOBinaryArrayOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMultidimensionalFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVisibilityChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOCompoundReductions.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOReducer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOFunctorBroadcaster.hh
)

if(HOPS_USE_FFTW3)
    list(APPEND OPERATOR_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMultidimensionalFastFourierTransformFFTW.hh
    )
endif(HOPS_USE_FFTW3)

#list source files #############################################################

set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOFastFourierTransform.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOVisibilityChannelizer.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(OPERATOR_LIBS MHOMessage MHOMath)

if(HOPS_USE_FFTW3)
    list(APPEND OPERATOR_LIBS ${FFTW_LIBRARIES})
endif(HOPS_USE_FFTW3)

add_library (MHOOperators SHARED ${OPERATOR_SOURCEFILES})
target_link_libraries (MHOOperators ${OPERATOR_LIBS})

hops_install_headers (${OPERATOR_HEADERFILES})
hops_install_libraries (MHOOperators )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
