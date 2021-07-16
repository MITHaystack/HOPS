#list header files #############################################################
set (CALIBRATION_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_ChannelizedRotationFunctor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_VisibilityChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_WeightChannelizer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_NormFX.hh
)

#list source files #############################################################
set( CALIBRATION_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_ChannelizedRotationFunctor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_VisibilityChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_WeightChannelizer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_NormFX.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(CALIBRATION_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers)

#link against old fourfit libraries (temporary measure while we try to debug/transition)
if(HOPS_DEV_USE_OLD)

    #declare header paths ##########################################################
    set(MK4UTIL_RELPATH "../../c_src/mk4util")
    set(DFIO_RELPATH "../../c_src/dfio")
    set(VEX_RELPATH "../../c_src/vex")
    set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol")
    set(FFCORE_RELPATH "../../c_src/fourfit_libs/ffcore")
    set(FFMATH_RELPATH "../../c_src/fourfit_libs/ffmath")
    set(FFIO_RELPATH "../../c_src/fourfit_libs/ffio")

    list(APPEND RELPATH_LIST
        ${MK4UTIL_RELPATH} ${AFIO_RELPATH} ${DFIO_RELPATH} ${VEX_RELPATH}
        ${FFCONTROL_RELPATH} ${FFCORE_RELPATH} ${FFIO_RELPATH} ${FFMATH_RELPATH}
    )

    #include library headers
    foreach(path ${RELPATH_LIST})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)


    #build/link/install library ####################################################
    list(APPEND CALIBRATION_LIBS "mk4util" "afio"
                        "dfio" "vex"
                        "ffcontrol" "ffcore"
                        "ffio" "ffmath"
    )

    #also need fftw3
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND CALIBRATION_LIBS ${FFTW_LIBRARIES})

endif(HOPS_DEV_USE_OLD)


#compile and install library ###################################################
add_library (MHO_Calibration SHARED ${CALIBRATION_SOURCEFILES})
target_link_libraries (MHO_Calibration ${CALIBRATION_LIBS})

hops_install_headers (${CALIBRATION_HEADERFILES})
hops_install_libraries (MHO_Calibration )

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
