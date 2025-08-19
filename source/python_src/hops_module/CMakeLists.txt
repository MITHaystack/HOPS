#python package setup ##########################################################
if (PY_EXE)

    # Install Python module packages directly using CMake
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hops" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
            
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hops_io" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
            
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hops_visualization" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)

endif(PY_EXE)
