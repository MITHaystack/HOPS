if(PY_EXE)

    if(HOPS_PYPI_MANAGE_DEPS)
        
         #if the user chooses to enable HOPS_PYPI_MANAGE_DEPS, then we can use pip to pull/install the python dependencies
         #for us at time of installation, but we don't do this automatically 
         set(REQUIREMENTS_TXT_IN "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt")
         set(REQUIREMENTS_TXT "${CMAKE_CURRENT_BINARY_DIR}/requirements.txt")
         configure_file(${REQUIREMENTS_TXT_IN} ${REQUIREMENTS_TXT})
         install(CODE "execute_process(COMMAND ${PY_EXE} -m pip install --upgrade --target=${PYTHON_MODULE_INSTALL_DIR} -r ${REQUIREMENTS_TXT} )")

    else(HOPS_PYPI_MANAGE_DEPS)

        #look for our minimal set of python dependencies and warn user if they are not found
        set(PYTHON_PACKAGES "numpy" "scipy" "matplotlib")

        foreach(pkg IN LISTS PYTHON_PACKAGES)
            execute_process(
                COMMAND ${PY_EXE} -c "import ${pkg}"
                RESULT_VARIABLE _status
                OUTPUT_QUIET
                ERROR_QUIET
            )
            if(NOT _status EQUAL 0)
                message(WARNING "Python package '${pkg}' not found. You may need to install it, or toggle HOPS_PYPI_MANAGE_DEPS to ON ")
                set(MISSING_PYTHON_PACKAGES TRUE)
            endif()
        endforeach()

        #tell user how to install the missing dependencies
        if(MISSING_PYTHON_PACKAGES)
            message(STATUS "Not all python dependencies were found, some python extension functionality may be missing.")
            message(STATUS "You can install the missing packages automatically by enabling HOPS_PYPI_MANAGE_DEPS or manually with:")
            message(STATUS "  pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt")
        endif()

    endif(HOPS_PYPI_MANAGE_DEPS)

    if(HOPS3_PYTHON_EXTRAS AND NOT HOPS3_USE_CXX)
        add_subdirectory(mk4_module)
        add_subdirectory(vpal_module)
        add_subdirectory(ffcontrol_module)
    endif()

    if(HOPS_ENABLE_TEST)
        enable_testing()
        add_subdirectory(test)
    endif(HOPS_ENABLE_TEST)

    add_subdirectory(hops_module)
    add_subdirectory(scripts)
    add_subdirectory(plugins)

endif(PY_EXE)
