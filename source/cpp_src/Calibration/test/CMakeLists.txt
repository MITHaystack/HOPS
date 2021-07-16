#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

#list of c++ libraries
set(TEST_LIBS MHO_Message MHO_Math MHO_Operators MHO_Containers MHO_Calibration)

#link against old fourfit libraries (temporary measure while we try to debug/transition)
if(HOPS_DEV_USE_OLD)

    #declare header paths ##########################################################
    set(MK4UTIL_RELPATH "../../../c_src/mk4util")
    set(DFIO_RELPATH "../../../c_src/dfio")
    set(VEX_RELPATH "../../../c_src/vex")
    set(FFCONTROL_RELPATH "../../../c_src/fourfit_libs/ffcontrol")
    set(FFCORE_RELPATH "../../../c_src/fourfit_libs/ffcore")
    set(FFMATH_RELPATH "../../../c_src/fourfit_libs/ffmath")
    set(FFIO_RELPATH "../../../c_src/fourfit_libs/ffio")

    list(APPEND RELPATH_LIST
        ${MK4UTIL_RELPATH} ${AFIO_RELPATH} ${DFIO_RELPATH} ${VEX_RELPATH}
        ${FFCONTROL_RELPATH} ${FFCORE_RELPATH} ${FFIO_RELPATH} ${FFMATH_RELPATH}
    )

    #include library headers
    foreach(path ${RELPATH_LIST})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)


    #build/link/install library ####################################################
    list(APPEND TEST_LIBS "mk4util" "afio"
                        "dfio" "vex"
                        "ffcontrol" "ffcore"
                        "ffio" "ffmath"
    )

    #also need fftw3
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND TEST_LIBS ${FFTW_LIBRARIES})

endif(HOPS_DEV_USE_OLD)


#compile and install test executables
set(SOURCE_BASENAMES
    TestNormFX
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
