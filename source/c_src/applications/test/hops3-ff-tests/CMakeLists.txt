#configure environment set-up script
find_program (BASH_PROGRAM bash REQUIRED)

if(BASH_PROGRAM)

        set(TEST_SCRIPT_BASENAMES "")

        list(APPEND TEST_SCRIPT_BASENAMES
            chk_ff_2836
            chk_ff_2843
            chk_ff_3372
            chk_ff_3413
            chk_ff_3756
            chk_ff_3571
            chk_ff_3772pb
            chk_baselines
            chk_alist
            chk_adump
            chk_aedit
            chk_min_weight
            chk_passband
            chk_notches
            chk_fringex
            chk_average
            chk_avefix
            chk_cofit
            chk_search
            chk_fourmer
            chk_frmrsrch
            chk_notches_xf
            chk_ps2pdf
            #the following tests fail (either as c or c++)
            #chk_hdlinks -- this one fails (probably we missing the perl script hops_data_links.pl)
            #chk_ff_display
            #chk_ff_dump
            #chk_ff_ps2pdf
            #chk_flagging
            #chk_fsumm
        )

        foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
            configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
            add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
            set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS TESTDATA_UNPACK)
            set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
        endforeach( BASENAME )

endif(BASH_PROGRAM)
