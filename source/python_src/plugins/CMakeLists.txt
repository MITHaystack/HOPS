if (PY_EXE)
    
    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/example1.py" "${CMAKE_CURRENT_BINARY_DIR}/example1.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/example1.py" DESTINATION ${PLUGINS_INSTALL_DIR} )

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/example2.py" "${CMAKE_CURRENT_BINARY_DIR}/example2.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/example2.py" DESTINATION ${PLUGINS_INSTALL_DIR} )

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/example3.py" "${CMAKE_CURRENT_BINARY_DIR}/example3.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/example3.py" DESTINATION ${PLUGINS_INSTALL_DIR} )

endif (PY_EXE)
