#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

option( HOPS_ENABLE_REMOTE_TEST_DATA "Download remote data for developer tests." ON)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/hops4_testdata.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/hops4_testdata.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/testdata)

        message(STATUS "Downloading and unpacking test data.")

        add_custom_target( TESTDATA_DOWNLOAD ALL
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        add_custom_target( TESTDATA_UNPACK ALL
          COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          DEPENDS TESTDATA_DOWNLOAD
        )

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
        endforeach( BASENAME )

   endif(HOPS_ENABLE_REMOTE_TEST_DATA)

endif(BASH_PROGRAM)
