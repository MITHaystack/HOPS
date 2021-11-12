#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

option( HOPS_ENABLE_REMOTE_TEST_DATA "Download remote data for developer tests." OFF)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/hops4_testdata.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/testdata)

        add_test(NAME TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        add_test(NAME TESTDATA_UNPACK
            COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_tests_properties(TESTDATA_UNPACK PROPERTIES DEPENDS TESTDATA_DOWNLOAD)

        set(TEST_SCRIPT_BASENAMES
            chk_ff_2836
            chk_ff_2843
            chk_ff_3372
            chk_ff_3413
            chk_ff_3571
        )

        foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
            configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
            add_test(NAME ${BASENAME}_test COMMAND  ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
            set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS TESTDATA_UNPACK)
        endforeach( BASENAME )

   endif(HOPS_ENABLE_REMOTE_TEST_DATA)

endif(BASH_PROGRAM)
