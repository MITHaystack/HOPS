#configure environment set-up script
find_program (BASH_PROGRAM bash)

if(BASH_PROGRAM)

    set(TEST_SCRIPT_BASENAMES
        chk_simplefringesearch
    )

    foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
        add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
        set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS DIFX_TESTDATA_UNPACK)
        set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
    endforeach( BASENAME )

endif(BASH_PROGRAM)
