if (HOPS_ENABLE_TEST)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Array/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/include)

    set (HOPS_TEST_LIBS HCore)

    set(SOURCE_BASENAMES
        TestMultiTypeMap
        TestTimeStampConverter
    )

    foreach( BASENAME ${SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    endforeach( BASENAME )

    hops_install_executables( ${SOURCE_BASENAMES} )

endif (HOPS_ENABLE_TEST)
