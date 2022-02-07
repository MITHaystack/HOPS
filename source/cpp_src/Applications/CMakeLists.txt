set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
set(DFIO_RELPATH "../../c_src/dfio/include")
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

hops_internal_include_directories()

set(HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Operators MHO_Calibration MHO_MK4Interface)


#link against old fourfit libraries (temporary measure while we try to debug/transition)
if(HOPS_DEV_USE_OLD)

    #declare header paths ##########################################################
    set(FFCONTROL_RELPATH "../../c_src/fourfit_libs/ffcontrol")
    set(FFCORE_RELPATH "../../c_src/fourfit_libs/ffcore")
    set(FFMATH_RELPATH "../../c_src/fourfit_libs/ffmath")
    set(FFIO_RELPATH "../../c_src/fourfit_libs/ffio")
    set(FFSEARCH_RELPATH "../../c_src/fourfit_libs/ffsearch")

    list(APPEND RELPATH_LIST
        ${FFCONTROL_RELPATH} ${FFCORE_RELPATH} ${FFIO_RELPATH} ${FFMATH_RELPATH} ${FFSEARCH_RELPATH}
    )

    #include library headers
    foreach(path ${RELPATH_LIST})
        hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${path}/include)
    endforeach(path)


    #build/link/install library ####################################################
    set(OLD_DEV_TEST_LIBS "mk4util"
                        "dfio" "vex"
                        "ffcontrol" "ffcore"
                        "ffio" "ffmath" "ffsearch"
    )

    #also need fftw3
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
    list(APPEND OLD_DEV_TEST_LIBS ${FFTW_LIBRARIES})

endif(HOPS_DEV_USE_OLD)


set(SOURCE_BASENAMES
    TestSingleBandDelay
    TestMultiObjectStreaming
    TestConvertMk4Visibilities
    DumpFileObjectKeys
    ConvertMk4Data
    DumpFileObject
)

#for testing purposes only -- ROOT not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND HOPS_TEST_LIBS ${ROOT_LIBRARIES})
endif()


foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )



#link against old fourfit libraries (temporary measure while we try to debug/transition)
if(HOPS_DEV_USE_OLD)

    list(APPEND OLD_DEV_SOURCE_BASENAMES
        TestNormFX
    )

    foreach( BASENAME ${OLD_DEV_SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS} ${OLD_DEV_TEST_LIBS})
    endforeach( BASENAME )

    hops_install_executables( ${OLD_DEV_SOURCE_BASENAMES} )

    endif(HOPS_DEV_USE_OLD)

# 
# if(HOPS_USE_DIFXIO)
# 
#     if(HOPS_USE_DIFXIO)
#         find_package(PkgConfig) # we use pkg-config to find difxio
#         pkg_search_module(DIFXIO REQUIRED difxio)
#     endif()
# 
#     hops_external_include_directories (${DIFXIO_INCLUDE_DIRS})
# 
#     list(APPEND DIFXIO_DEP_SOURCE_BASENAMES
#         difx2hops
#     )
# 
#     foreach( BASENAME ${DIFXIO_DEP_SOURCE_BASENAMES} )
#         add_executable( ${BASENAME} ${BASENAME}.cc )
#         target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS} ${DIFXIO_LDFLAGS})
#     endforeach( BASENAME )
# 
#     hops_install_executables( ${DIFXIO_DEP_SOURCE_BASENAMES} )
# 
# endif(HOPS_USE_DIFXIO)
