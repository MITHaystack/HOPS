#configure environment set-up script
find_program (BASH_PROGRAM bash REQUIRED)
find_program (TAR_PROGRAM tar REQUIRED)
find_program (WGET_PROGRAM wget REQUIRED)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/hops4_testdata.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/testdata)

        add_test(NAME TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        set(DIFX_REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/vt9105.tar.gz")
        set(DIFX_DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)
        set(DIFX_DATADIR ${CMAKE_CURRENT_BINARY_DIR}/vt9105)

        add_test(NAME DIFX_TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${DIFX_REMOTE_DATA} -O ${DIFX_DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    endif(HOPS_ENABLE_REMOTE_TEST_DATA)

    add_test(NAME TESTDATA_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(HOPS_ENABLE_REMOTE_TEST_DATA)
        set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_DOWNLOAD)
    endif(HOPS_ENABLE_REMOTE_TEST_DATA)

    add_test(NAME DIFX_TESTDATA_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DIFX_DATA_TARBALL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(HOPS_ENABLE_REMOTE_TEST_DATA)
        set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_DOWNLOAD)
    endif(HOPS_ENABLE_REMOTE_TEST_DATA)

    #configure the environment script
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/chk_cmake_env.sh ${CMAKE_CURRENT_BINARY_DIR}/chk_env.sh @ONLY)


endif(BASH_PROGRAM)
