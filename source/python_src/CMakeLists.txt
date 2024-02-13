if(PY_EXE)
    
    #just wholesale copy the entire python source directory into the build directory
    #because the pythonic wizards who wrote setuptools like to pollute the source
    #directory with egg.info garbage
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    #need this target to refresh our copy if anything in the source has changed
    # add_custom_target( hops4py_copy_src ALL
    #                    COMMAND ${CMAKE_COMMAND} -E copy_directory
    #                    "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    # )

    add_custom_target( hops4py_copy_src ALL
                       COMMAND cp -r "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    )

    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup.py.in")
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

endif(PY_EXE)
