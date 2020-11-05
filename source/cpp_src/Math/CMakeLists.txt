#list header files #############################################################
set (MATH_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOBitReversalPermutation.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOFastFourierTransformUtilities.hh
)


#list source files #############################################################
set( MATH_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOBitReversalPermutation.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOFastFourierTransformUtilities.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(MATH_LIBS MHOMessage)

add_library (MHOMath SHARED ${MATH_SOURCEFILES})
target_link_libraries (MHOMath ${MATH_LIBS})

hops_install_headers (${MATH_HEADERFILES})
hops_install_libraries (MHOMath )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
