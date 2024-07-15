if (PY_EXE)

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test.py.in" "${CMAKE_CURRENT_BINARY_DIR}/test.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/test.py" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_hops_access.py.in" "${CMAKE_CURRENT_BINARY_DIR}/test_hops_access.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/test_hops_access.py" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_hops_fringe.py.in" "${CMAKE_CURRENT_BINARY_DIR}/test_hops_fringe.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/test_hops_fringe.py" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/compjsonpdd.py.in" "${CMAKE_CURRENT_BINARY_DIR}/compjsonpdd.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/compjsonpdd.py" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pdd2json.py.in" "${CMAKE_CURRENT_BINARY_DIR}/pdd2json.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pdd2json.py" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pddjson2fplot.py.in" "${CMAKE_CURRENT_BINARY_DIR}/pddjson2fplot.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pddjson2fplot.py" DESTINATION ${BIN_INSTALL_DIR})

    if(HOPS3_PYTHON_EXTRAS)
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/batch_fourfit.py.in" "${CMAKE_CURRENT_BINARY_DIR}/batch_fourfit.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/batch_fourfit.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ffres2pcp.py.in" "${CMAKE_CURRENT_BINARY_DIR}/ffres2pcp.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/ffres2pcp.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/fourphase.py.in" "${CMAKE_CURRENT_BINARY_DIR}/fourphase.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/fourphase.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/phase_resid.py.in" "${CMAKE_CURRENT_BINARY_DIR}/phase_resid.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/phase_resid.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/summarize_report.py.in" "${CMAKE_CURRENT_BINARY_DIR}/summarize_report.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/summarize_report.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vgoscf_generate.py.in" "${CMAKE_CURRENT_BINARY_DIR}/vgoscf_generate.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/vgoscf_generate.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pcc_generate.py.in" "${CMAKE_CURRENT_BINARY_DIR}/pcc_generate.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pcc_generate.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pcc_plot.py.in" "${CMAKE_CURRENT_BINARY_DIR}/pcc_plot.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pcc_plot.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/pcc_select.py.in" "${CMAKE_CURRENT_BINARY_DIR}/pcc_select.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/pcc_select.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/select_bandpols.py.in" "${CMAKE_CURRENT_BINARY_DIR}/select_bandpols.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/select_bandpols.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generate_session_summary.py.in" "${CMAKE_CURRENT_BINARY_DIR}/generate_session_summary.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/generate_session_summary.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/compare_session.py.in" "${CMAKE_CURRENT_BINARY_DIR}/compare_session.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/compare_session.py" DESTINATION ${BIN_INSTALL_DIR})

        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/compare_session2.py.in" "${CMAKE_CURRENT_BINARY_DIR}/compare_session2.py" @ONLY)
        install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/compare_session2.py" DESTINATION ${BIN_INSTALL_DIR})

    endif(HOPS3_PYTHON_EXTRAS)

endif (PY_EXE)
