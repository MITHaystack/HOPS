#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (TAR_PROGRAM tar)
find_program (WGET_PROGRAM wget)

if(BASH_PROGRAM)

    if(HOPS_ENABLE_REMOTE_TEST_DATA)

        set(TEST_SCRIPT_BASENAMES
            chk_difx2hops
            chk_d2h_vs_d2m4
        )

        foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
            configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
            add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
            set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS DIFX_TESTDATA_UNPACK)
            set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
        endforeach( BASENAME )

   endif(HOPS_ENABLE_REMOTE_TEST_DATA)

endif(BASH_PROGRAM)
