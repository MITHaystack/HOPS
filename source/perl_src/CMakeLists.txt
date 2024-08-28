if(PERL_FOUND)

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/hops_data_links.pl.in" "${CMAKE_CURRENT_BINARY_DIR}/hops_data_links.pl" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/hops_data_links.pl" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/adhoc-baseline.pl" "${CMAKE_CURRENT_BINARY_DIR}/adhoc-baseline.pl" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/adhoc-baseline.pl" DESTINATION ${BIN_INSTALL_DIR})
    
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/print_adhoc_lines.pl" "${CMAKE_CURRENT_BINARY_DIR}/print_adhoc_lines.pl" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/print_adhoc_lines.pl" DESTINATION ${BIN_INSTALL_DIR})

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/compare-baselines-v6.pl" "${CMAKE_CURRENT_BINARY_DIR}/compare-baselines-v6.pl" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/compare-baselines-v6.pl" DESTINATION ${BIN_INSTALL_DIR})

endif (PERL_FOUND)
