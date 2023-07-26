#configure environment set-up script
find_program (BASH_PROGRAM bash REQUIRED)
find_program (TAR_PROGRAM tar REQUIRED)

if(BASH_PROGRAM)

    if(HOPS_USE_CACHED_TEST_DATA)

        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)

        add_test(NAME TESTDATA_COPY
          COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/hops4_testdata.tar.gz ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        set(DATA_TARBALL2 ${CMAKE_CURRENT_BINARY_DIR}/3764.tar.gz)

        add_test(NAME TESTDATA2_COPY
          COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/3764.tar.gz ${DATA_TARBALL2}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        set(DATA_TARBALL3 ${CMAKE_CURRENT_BINARY_DIR}/3593.tar.gz)

        add_test(NAME TESTDATA3_COPY
          COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/3593.tar.gz ${DATA_TARBALL3}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        set(DIFX_DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)

        add_test(NAME DIFX_TESTDATA_COPY
          COMMAND cp ${HOPS_CACHED_TEST_DATADIR}/vt9105.tar.gz ${DIFX_DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

    endif(HOPS_USE_CACHED_TEST_DATA)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        find_program (WGET_PROGRAM wget REQUIRED)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/hops4_testdata.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/testdata)

        add_test(NAME TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        set(REMOTE_DATA2 "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/3764.tar.gz")
        set(DATA_TARBALL2 ${CMAKE_CURRENT_BINARY_DIR}/3764.tar.gz)
        set(DATADIR2 ${CMAKE_CURRENT_BINARY_DIR}/3764)

        add_test(NAME DATA2_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA2} -O ${DATA_TARBALL2}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        set(REMOTE_DATA3 "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/3593.tar.gz")
        set(DATA_TARBALL3 ${CMAKE_CURRENT_BINARY_DIR}/3593.tar.gz)
        set(DATADIR3 ${CMAKE_CURRENT_BINARY_DIR}/3593)

        add_test(NAME DATA3_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA3} -O ${DATA_TARBALL3}
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
    
    add_test(NAME TESTDATA2_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL2}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_test(NAME TESTDATA3_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL3}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    add_test(NAME DIFX_TESTDATA_UNPACK
        COMMAND ${TAR_PROGRAM} -xzf ${DIFX_DATA_TARBALL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(HOPS_USE_CACHED_TEST_DATA)
        set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_COPY)
        set_tests_properties(TESTDATA2_UNPACK PROPERTIES DEPENDS TESTDATA2_COPY)
        set_tests_properties(TESTDATA3_UNPACK PROPERTIES DEPENDS TESTDATA3_COPY)
        set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_DOWNLOAD)
    endif(HOPS_USE_CACHED_TEST_DATA)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)
        set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_DOWNLOAD)
        set_tests_properties(TESTDATA2_UNPACK PROPERTIES DEPENDS TESTDATA2_DOWNLOAD)
        set_tests_properties(TESTDATA3_UNPACK PROPERTIES DEPENDS TESTDATA3_DOWNLOAD)
        set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_COPY)
    endif(HOPS_ENABLE_REMOTE_TEST_DATA)


    #configure the environment script
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/chk_cmake_env.sh ${CMAKE_CURRENT_BINARY_DIR}/chk_env.sh @ONLY)


endif(BASH_PROGRAM)
