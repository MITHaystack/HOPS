if(HDF5_FOUND)
    hops_internal_include_directories()

    set( HOPS_HDF5_LIBS )

    set(SOURCE_BASENAMES
        hops2hdf5
    )

    hops_external_include_directories(${HDF5_INCLUDE_DIRS})
    hops_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
    list(APPEND HOPS_HDF5_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_HDF5Interface ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARIES})

    foreach( BASENAME ${SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_HDF5_LIBS})
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME} DESTINATION ${BIN_INSTALL_DIR})
    endforeach( BASENAME )

endif(HDF5_FOUND)
