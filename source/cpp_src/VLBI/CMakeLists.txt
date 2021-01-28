#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Visibilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelizedVisibilities.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelizedRotationFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityChannelizer.hh
)

#list source files #############################################################
set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VisibilityChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelizedRotationFunctor.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(VLBI_LIBS MHO_Message MHO_Math MHO_Operators)

add_library (MHO_VLBI SHARED ${OPERATOR_SOURCEFILES})
target_link_libraries (MHO_VLBI ${VLBI_LIBS})

hops_install_headers (${OPERATOR_HEADERFILES})
hops_install_libraries (MHO_VLBI )

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
