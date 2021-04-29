#list header files #############################################################
set (CALIBRATION_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelizedRotationFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityChannelizer.hh
)

#list source files #############################################################
set( CALIBRATION_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VisibilityChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelizedRotationFunctor.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(CALIBRATION_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers)

add_library (MHO_Calibration SHARED ${CALIBRATION_SOURCEFILES})
target_link_libraries (MHO_Calibration ${CALIBRATION_LIBS})

hops_install_headers (${CALIBRATION_HEADERFILES})
hops_install_libraries (MHO_Calibration )

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
