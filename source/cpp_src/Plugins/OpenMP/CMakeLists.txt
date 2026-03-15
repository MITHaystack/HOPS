if(HOPS_USE_OPENMP)

if(OpenMP_CXX_FOUND)

    #declare header paths ##########################################################
    hops_internal_include_directories()
    hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

    #list header files #############################################################
    set (OPENMP_PLUGIN_HEADERFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_MBDelaySearchOpenMP.hh
        #${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_IonosphericFringeFitterOpenMP.hh
    )

    #list source files #############################################################
    set( OPENMP_PLUGIN_SOURCEFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_MBDelaySearchOpenMP.cc
        #${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_IonosphericFringeFitterOpenMP.cc
    )

    #set libraries to link against ###########################
    set(HOPS_OMP_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration OpenMP::OpenMP_CXX)

    add_library(MHO_OpenMPPlugin SHARED ${OPENMP_PLUGIN_SOURCEFILES})
    target_link_libraries(MHO_OpenMPPlugin ${HOPS_OMP_LIBS})

    hops_install_headers (${OPENMP_PLUGIN_HEADERFILES})
    hops_install_libraries (MHO_OpenMPPlugin)

    # if (HOPS_ENABLE_TEST)
    #     add_subdirectory(test)
    # endif (HOPS_ENABLE_TEST)

endif(OpenMP_CXX_FOUND)

endif(HOPS_USE_OPENMP)
