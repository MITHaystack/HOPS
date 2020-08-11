if (HOPS_ENABLE_TEST)

    set(MK4STUB_RELPATH "../../c_src/mk4stub/include")
    set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
    set(DFIO_RELPATH "../../c_src/dfio/include")
    set(VEX_RELPATH "../../c_src/vex/include")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4STUB_RELPATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})


    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Array/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../MK4Interface/include)




    set (HOPS_TEST_LIBS HCore HMK4Interface mk4stub mk4util dfio vex )

    set(SOURCE_BASENAMES
        TestMultiTypeMap
        TestTimeStampConverter
        TestScalarContainer
        TestVectorContainer
        TestTensorContainer
        TestMK4Interface
        TestMK4CorelInterface
    )

    foreach( BASENAME ${SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    endforeach( BASENAME )

    hops_install_executables( ${SOURCE_BASENAMES} )

endif (HOPS_ENABLE_TEST)
