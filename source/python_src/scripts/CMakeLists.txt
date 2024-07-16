if (PY_EXE)


    set(SOURCE_BASENAMES
        hops_version
        test_hops_access
        test_hops_fringe
        compjsonpdd
        pdd2json
        pddjson2fplot
    )

    if(HOPS3_PYTHON_EXTRAS)
        
        #install additional scripts
        list(APPEND SOURCE_BASENAMES
            az_el_amp_plot
            batch_fourfit_mixedmode
            batch_fourfit
            channel_amplitude_compare
            channel_amplitude
            channel_pcal_amp
            channel_phase_resid
            compare_control_files
            compare_pcc_delay
            compare_pol_hands
            compare_session
            ffres2pcp_mixedmode
            ffres2pcp
            fourphase_mixedmode
            fourphase
            generate_session_summary
            get_ionex_dtec_bounds
            parallactic_plots
            pcc_generate
            pcc_plot
            pcc_select
            phase_resid_mixedmode
            phase_resid
            plot_pcal_amps
            select_bandpols
            summarize_report
            vgoscf_generate
        )

    endif(HOPS3_PYTHON_EXTRAS)
    
    #configure and install scripts
    foreach( BASENAME ${SOURCE_BASENAMES} )
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.py.in" "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py" DESTINATION ${BIN_INSTALL_DIR})
    endforeach( BASENAME )
    

endif (PY_EXE)
