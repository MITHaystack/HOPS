

#python package setup ##########################################################
if (PY_EXE)

    # Install Python module packages directly using CMake
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/afio" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
            
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hops_test" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
            
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/mk4io" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
            
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vexpy" 
            DESTINATION "${PYTHON_MODULE_INSTALL_DIR}"
            FILES_MATCHING 
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
    
endif(PY_EXE)
