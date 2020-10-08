if (HOPS_ENABLE_TEST)

    set(MK4UTIL_RELPATH "../../c_src/mk4util/include")
    set(DFIO_RELPATH "../../c_src/dfio/include")
    set(VEX_RELPATH "../../c_src/vex/include")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Array/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../MK4Interface/include)

    set (HOPS_TEST_LIBS HCore HMK4Interface mk4util dfio vex )

    set(SOURCE_BASENAMES
        TestMultiTypeMap
        TestTimeStampConverter
        TestArrayWrapper
        TestScalarContainer
        TestVectorContainer
        TestMK4Interface
        TestMK4CorelInterface
        TestMK4VexInterface
        TestMK4Import
    )

    #for testing purposes only -- not intended to be a dependency
    if (HOSE_USE_ROOT)
        include(FindROOT)
        find_package(ROOT REQUIRED)
        include_directories(${ROOT_INCLUDE_DIRS})
        add_compile_definitions(USE_ROOT)
        list(APPEND HOPS_TEST_LIBS ${ROOT_LIBRARIES})
        list(APPEND SOURCE_BASENAMES TestTensorContainer)
    endif()


    foreach( BASENAME ${SOURCE_BASENAMES} )
        add_executable( ${BASENAME} ${BASENAME}.cc )
        target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    endforeach( BASENAME )

    hops_install_executables( ${SOURCE_BASENAMES} )

endif (HOPS_ENABLE_TEST)
