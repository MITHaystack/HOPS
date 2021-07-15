#configure environment set-up script
find_program (BASH_PROGRAM bash)

if(BASH_PROGRAM)
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/chk_ff_3571.sh ${CMAKE_CURRENT_BINARY_DIR}/chk_ff_3571.sh @ONLY)
    add_test(NAME chk_ff_3571_test COMMAND  ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/chk_ff_3571.sh )
endif(BASH_PROGRAM)
