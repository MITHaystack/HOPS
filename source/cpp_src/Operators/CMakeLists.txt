#list header files #############################################################
set (OPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHOVisibilityChannelizer.hh
)

#list source files #############################################################

set( OPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHOVisibilityChannelizer.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(OPERATOR_LIBS MHOMessage)


add_library (MHOOperators SHARED ${OPERATOR_SOURCEFILES})
target_link_libraries (MHOOperators ${OPERATOR_LIBS})

hops_install_headers (${OPERATOR_HEADERFILES})
hops_install_libraries (MHOOperators )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
