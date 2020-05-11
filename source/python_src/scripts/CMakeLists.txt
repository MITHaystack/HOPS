if (PYTHON_EXECUTABLE)

    #configure and install scripts
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test.py.in" "${CMAKE_CURRENT_BINARY_DIR}/test.py" @ONLY)
    install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/test.py" DESTINATION ${BIN_INSTALL_DIR})

endif (PYTHON_EXECUTABLE)
