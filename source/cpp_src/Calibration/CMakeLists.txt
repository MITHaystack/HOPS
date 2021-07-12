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


# #link against old fourfit libraries (temporary measure while we try to debug/transition)
# if(HOPS_DEV_USE_OLD)
#     find_package(FFTW REQUIRED)
#     hops_external_include_directories (${FFTW_INCLUDE_DIRS})
#     hops_add_cxxflag(HOPS_USE_FFTW3)
#     list(APPEND HOPS_TEST_LIBS ${FFTW_LIBRARIES})
# endif(HOPS_DEV_USE_OLD)



set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(DFIO_RELPATH "../../c_src/dfio/include")
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})







#compile and install library ###################################################
set(CALIBRATION_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers)

add_library (MHO_Calibration SHARED ${CALIBRATION_SOURCEFILES})
target_link_libraries (MHO_Calibration ${CALIBRATION_LIBS})

hops_install_headers (${CALIBRATION_HEADERFILES})
hops_install_libraries (MHO_Calibration )

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
