if(HDF5_FOUND)
    hops_internal_include_directories()

    set( HOPS_TEST_LIBS )

    set(SOURCE_BASENAMES
        TestHDF5Interface
        hops2hdf5
    )

    hops_external_include_directories(${HDF5_INCLUDE_DIRS})
    hops_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
    list(APPEND HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

    foreach( BASENAME ${SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
        add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
    endforeach( BASENAME )

    hops_install_test_executables( ${SOURCE_BASENAMES} )

endif(HDF5_FOUND)
