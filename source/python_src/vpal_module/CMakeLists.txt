

#python package setup ##########################################################
if (PY_EXE)

    # Install Python module package directly using CMake
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vpal" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
    
endif(PY_EXE)
