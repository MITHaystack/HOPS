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
            # generate_session_summary
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
            vex2xml
            hops4_read_example
            hops4_fplot_example
            hops4_fparam_example
            hops4_visplot_example
            hops4_fringe_summary
            hops3_fringe_summary
            fringe_vinspect
            compare_fringe_summary
            profile_plot
            profile_plot_alt
            profile_report
            run_pyfringe_example
            #imported from hops3
            alma-vex-defs # this script is needed at correlators to support correlations with ALMA
            est_manual_phases # this script is needed in analysis to estimate manual phase cal phases
            hops_vex2ovex # this script is needed to support making summary plots from aedit
        )

    endif(HOPS3_PYTHON_EXTRAS)

    #Self-locating sys.path bootstrap injected right after each tool's shebang, that way
    #every installed CLI tool can find the HOPS python modules relative to its own
    #location (bin/ -> ../${PYTHON_SITE_PREFIX}) without relying on a globally
    #exported PYTHONPATH. The path is resolved at runtime from __file__, so it should
    #stay correct if the install tree is relocated.
    #NOTE! if the PYTHON_SITE_PREFIX location changes relative location in future HOPS this will need to be modified
    set(HOPS_PY_PATH_BOOTSTRAP
"import os as _hops_os, sys as _hops_sys
_hops_sys.path.insert(0, _hops_os.path.realpath(_hops_os.path.join(_hops_os.path.dirname(_hops_os.path.abspath(__file__)), \"..\", \"${PYTHON_SITE_PREFIX}\")))
del _hops_os, _hops_sys
")

    #configure and install scripts
    foreach( BASENAME ${SOURCE_BASENAMES} )
        #configure_file (keeps the .py.in tracked as a reconfigure dependency),
        #then splice in the bootstrap locator in right after the shebang line.
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.py.in" "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py.in.configured" @ONLY)
        file(READ "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py.in.configured" _hops_script_body)
        string(REGEX MATCH "^[^\n]*\n" _hops_shebang "${_hops_script_body}")
        if(_hops_shebang)
            string(LENGTH "${_hops_shebang}" _hops_shebang_len)
            string(SUBSTRING "${_hops_script_body}" ${_hops_shebang_len} -1 _hops_rest)
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py" "${_hops_shebang}${HOPS_PY_PATH_BOOTSTRAP}${_hops_rest}")
        else()
            #no shebang found (unexpected): prepend the bootstrap to be safe.
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py" "${HOPS_PY_PATH_BOOTSTRAP}${_hops_script_body}")
        endif()
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.py" DESTINATION ${BIN_INSTALL_RELDIR})
    endforeach( BASENAME )

endif (PY_EXE)
