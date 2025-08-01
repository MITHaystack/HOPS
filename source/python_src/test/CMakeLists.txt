#configure environment set-up script
find_program (BASH_PROGRAM bash)

if(BASH_PROGRAM)

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    set(TEST_SCRIPT_BASENAMES "")

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_mk4io.py" "${CMAKE_CURRENT_BINARY_DIR}/test_mk4io.py" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_ffres2pcp.py" "${CMAKE_CURRENT_BINARY_DIR}/test_ffres2pcp.py" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_fourphase.py" "${CMAKE_CURRENT_BINARY_DIR}/test_fourphase.py" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_pcc_generate.py" "${CMAKE_CURRENT_BINARY_DIR}/test_pcc_generate.py" @ONLY)

    if(HOPS3_PYTHON_EXTRAS)
        list(APPEND TEST_SCRIPT_BASENAMES "vpal_test_harness3")
        list(APPEND TEST_SCRIPT_BASENAMES "vpal_test_harness4")
    endif(HOPS3_PYTHON_EXTRAS)

    foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh.in ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
        add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
        set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS TESTDATA3_UNPACK)
        set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
    endforeach( BASENAME )

endif(BASH_PROGRAM)
