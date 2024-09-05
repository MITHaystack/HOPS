if(PY_EXE)

    #just wholesale copy the entire python source directory into the build directory
    #because the pythonic wizards who wrote setuptools like to pollute the source
    #directory with egg.info garbage
    if( NOT ("${CMAKE_CURRENT_SOURCE_DIR}/" EQUAL  "${CMAKE_CURRENT_BINARY_DIR}/" ) )
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
    endif()

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    # add_custom_target( hops4py_copy_src ALL
    #                    COMMAND ${CMAKE_COMMAND} -E copy_directory
    #                    "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    # )

    #need this target to refresh our copy if anything in the source has changed
    if( NOT "${CMAKE_CURRENT_SOURCE_DIR}/" EQUAL  "${CMAKE_CURRENT_BINARY_DIR}/" )
        add_custom_target( hops4py_copy_src ALL
                           COMMAND cp -r "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
        )
    endif()

    #setup-no-deps.py.in assumes that the system already has the python dependencies
    #needed (numpy, matplotlib, future, scipy) and makes not attempt to manage or
    #install them, this is also useful for off-line installations
    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup-no-deps.py.in")
    if(HOPS_PYPI_MANAGE_DEPS)
        #if the user chooses, we can as pip to pull and install the python dependencies
        #for us at time of installation (but don't do this automatically)
        set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup-deps.py.in")
    endif(HOPS_PYPI_MANAGE_DEPS)

    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    #add a build target to catch problems before install (output not used)
    if(HOPS3_PYTHON_EXTRAS)
    add_custom_target(hops4py_build_target ALL
        COMMAND ${PY_EXE} ${SETUP_PY} build
        DEPENDS hops4py_copy_src
    )
    endif(HOPS3_PYTHON_EXTRAS)

    if("${PYTHON_SETUP}" STREQUAL "setuptools")
        #use pip/setuptools
        install(CODE "execute_process(COMMAND ${PY_EXE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} --upgrade --target=${PYTHON_MODULE_INSTALL_DIR} )")
    else()
        #use distutils
        install(CODE "execute_process( COMMAND ${PY_EXE} ${SETUP_PY} install --prefix=${CMAKE_INSTALL_PREFIX} )")
    endif()

    if(HOPS3_PYTHON_EXTRAS AND NOT HOPS3_USE_CXX)
        add_subdirectory(mk4_module)
        add_subdirectory(vpal_module)
        add_subdirectory(ffcontrol_module)
    endif(HOPS3_PYTHON_EXTRAS AND NOT HOPS3_USE_CXX)

    if(HOPS_ENABLE_TEST)
        enable_testing()
        add_subdirectory(test)
    endif(HOPS_ENABLE_TEST)

    add_subdirectory(scripts)
    add_subdirectory(plugins)

endif(PY_EXE)
