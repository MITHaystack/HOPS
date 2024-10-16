#configure environment set-up script
find_program (BASH_PROGRAM bash)
find_program (BC_PROGRAM bc)

#awk/grep also needed
if(BASH_PROGRAM)

    set(TEST_SCRIPT_BASENAMES chk_m42h chk_simplefringesearch chk_simplefringesearch2) #chk_simplefringesearch2 has no python dependence just bash/bc

    #check relies on difxio -> hops conversion of data before it can be run
    if(BC_PROGRAM AND HOPS_USE_DIFXIO)
        list(APPEND TEST_SCRIPT_BASENAMES chk_dumpfileobjkeys)
    endif(BC_PROGRAM AND HOPS_USE_DIFXIO)

    if(PY_EXE)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/compjson.py" "${CMAKE_CURRENT_BINARY_DIR}/compjson.py" @ONLY)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/hopsobjdata.py" "${CMAKE_CURRENT_BINARY_DIR}/hopsobjdata.py" @ONLY)
        list(APPEND TEST_SCRIPT_BASENAMES 
            chk_convertvex2json 
            chk_converthops2json 
            chk_simplefringesearch3 
            chk_simplefringesearch4 
            chk_passband 
            chk_dsb1
            chk_geomixedmode
            chk_phs_jump
        )
    endif(PY_EXE)

    foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
        add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
        set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS DIFX_TESTDATA_UNPACK)
        set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
    endforeach( BASENAME )

endif(BASH_PROGRAM)
