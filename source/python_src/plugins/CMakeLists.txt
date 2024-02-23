if (PY_EXE)

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/mho_test2.py" "${CMAKE_CURRENT_BINARY_DIR}/mho_test2.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/mho_test2.py" DESTINATION ${PLUGINS_INSTALL_DIR} )

endif (PY_EXE)
