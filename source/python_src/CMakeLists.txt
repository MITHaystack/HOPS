#just wholesale copy the entire python source directory into the build directory 
#because the pythonic wizards who wrote setuptools like to pollute the source 
#directory with egg.info garbage
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")


if (PY_EXE)

    set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

    #need this target to refresh our copy if anything in the source has changed
    add_custom_target( hops4py_copy_src ALL
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/" "${CMAKE_CURRENT_BINARY_DIR}/"
    )

    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    add_custom_target(hops4py_build_target ALL 
        COMMAND ${PY_EXE} ${SETUP_PY} build
        DEPENDS mk4io_copy_src
    )

    if("${PYTHON_SETUP}" STREQUAL "setuptools")
        #use pip/setuptools
        add_custom_target(hops4py_install_target ALL 
            COMMAND ${PY_EXE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} --target=${PYTHON_MODULE_INSTALL_DIR}
            DEPENDS mk4io_build_target
        )

    else()
        #use distutils
        add_custom_target(hops4py_install_target ALL 
            COMMAND ${PY_EXE} ${SETUP_PY} install --prefix=${CMAKE_INSTALL_PREFIX} 
            DEPENDS mk4io_build_target
        )
    endif()

    add_subdirectory(scripts)
    add_subdirectory(mk4_module)
    
endif(PY_EXE)