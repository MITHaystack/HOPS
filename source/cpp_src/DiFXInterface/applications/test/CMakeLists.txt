#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(REMOTE_DATA "ftp://gemini.haystack.mit.edu/pub/barrettj/hops/vt9105.tar.gz")
        set(DATA_TARBALL ${CMAKE_CURRENT_BINARY_DIR}/vt9105.tar.gz)
        set(DATADIR ${CMAKE_CURRENT_BINARY_DIR}/vt9105)
        set(CMAKE_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR})

        add_test(NAME DIFX_TESTDATA_DOWNLOAD
          COMMAND ${WGET_PROGRAM} -q ${REMOTE_DATA} -O ${DATA_TARBALL}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        add_test(NAME DIFX_TESTDATA_UNPACK
            COMMAND ${TAR_PROGRAM} -xzf ${DATA_TARBALL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
        set_tests_properties(DIFX_TESTDATA_UNPACK PROPERTIES DEPENDS DIFX_TESTDATA_DOWNLOAD)


        set(TEST_SCRIPT_BASENAMES
            chk_difx2hops
            chk_d2h_vs_d2m4
        )

        #configure the environment
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/chk_cmake_env.sh ${CMAKE_CURRENT_BINARY_DIR}/chk_env.sh @ONLY)

        foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
            configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
            add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
            set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS DIFX_TESTDATA_UNPACK)
            set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${CMAKE_CURRENT_BINARY_DIR}")
        endforeach( BASENAME )

   endif(HOPS_ENABLE_REMOTE_TEST_DATA)

endif(BASH_PROGRAM)
