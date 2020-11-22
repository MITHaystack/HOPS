#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVisibilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOChannelizedVisibilities.hh
    #${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVRotationFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVisibilityChannelizer.hh
)

#list source files #############################################################
set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOVisibilityChannelizer.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(VLBI_LIBS MHOMessage MHOMath MHOOperators)

add_library (MHOVLBI SHARED ${OPERATOR_SOURCEFILES})
target_link_libraries (MHOVLBI ${VLBI_LIBS})

hops_install_headers (${OPERATOR_HEADERFILES})
hops_install_libraries (MHOVLBI )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
