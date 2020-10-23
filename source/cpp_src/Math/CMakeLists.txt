#add an option on whether or not to use FFTW3
option (HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms" OFF)
if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
endif (HOPS_USE_FFTW3)

#list header files #############################################################
set (MATH_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOBitReversalPermutation.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOFastFourierTransform.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOFastFourierTransformUtilities.hh

    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMultidimensionalFastFourierTransform.hh
)

if(HOPS_USE_FFTW3)
    list(APPEND MATH_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOMultidimensionalFastFourierTransformFFTW.hh
    )
endif(HOPS_USE_FFTW3)

#list source files #############################################################

set( MATH_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOBitReversalPermutation.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOFastFourierTransform.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOFastFourierTransformUtilities.cc
)

#declare header paths ##########################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Messaging/include)

#compile and install library ###################################################
set(MATH_LIBS MHOMessage)

if(HOPS_USE_FFTW3)
    list(APPEND MATH_LIBS ${FFTW_LIBRARIES})
endif(HOPS_USE_FFTW3)

add_library (MHOMath SHARED ${MATH_SOURCEFILES})
target_link_libraries (MHOMath ${MATH_LIBS})

hops_install_headers (${MATH_HEADERFILES})
hops_install_libraries (MHOMath )
