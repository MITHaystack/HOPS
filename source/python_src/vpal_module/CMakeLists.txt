

#python package setup ##########################################################
if (PY_EXE)

    #file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    #need this target to refresh our copy if anything in the source has changed
    add_custom_target( vpal_copy_src ALL
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    )

    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    #setup-no-deps.py.in assumes that the system already has the python dependencies
    #needed (numpy, matplotlib, future, scipy) and makes not attempt to manage or
    #install them, this is also useful for off-line installations
    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup-no-deps.py.in")
    if(HOPS_PYPI_MANAGE_DEPS)
        #if the user chooses, we can as pip to pull and install the python dependencies
        #for us at time of installation (but don't do this automatically)
        set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup-deps.py.in")
    endif(HOPS_PYPI_MANAGE_DEPS)
    configure_file(${SETUP_PY_IN} ${SETUP_PY})


    #add a build target to catch problems before install (output not used)
    add_custom_target(vpal_build_target ALL 
        COMMAND ${PY_EXE} ${SETUP_PY} build
        DEPENDS hops4py_copy_src vpal_copy_src
    )

    if("${PYTHON_SETUP}" STREQUAL "setuptools")
        #use pip/setuptools
        install(CODE "execute_process(COMMAND ${PY_EXE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} --upgrade --target=${PYTHON_MODULE_INSTALL_DIR} )")
    else()
        #use distutils
        install(CODE "execute_process( COMMAND ${PY_EXE} ${SETUP_PY} install --prefix=${CMAKE_INSTALL_PREFIX} )")
    endif()
    
endif(PY_EXE)
