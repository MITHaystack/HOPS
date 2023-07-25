#just wholesale copy the entire python source directory into the build directory 
#because the pythonic wizards who wrote setuptools like to pollute the source 
#directory with egg.info garbage
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

set(abs_srcdir "${CMAKE_CURRENT_BINARY_DIR}")

if (PY_EXE)

    set(SETUP_PY_IN "${CMAKE_CURRENT_BINARY_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(DEPS        "${CMAKE_CURRENT_BINARY_DIR}/hops/__init__.py")
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/hops4py_timestamp")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${PY_EXE} ${SETUP_PY} build
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS ${DEPS})

    add_custom_target(hops4py_target ALL DEPENDS ${OUTPUT})

    if("${PYTHON_SETUP}" STREQUAL "setuptools")
        #use setuptools/pip
        install(CODE "execute_process(COMMAND ${PY_EXE} -m pip install ${CMAKE_CURRENT_BINARY_DIR} --upgrade --target=${PYTHON_MODULE_INSTALL_DIR} )")
    else()
        #use distutils
        install(CODE "execute_process( COMMAND ${PY_EXE} ${SETUP_PY} install --prefix=${CMAKE_INSTALL_PREFIX} )")
    endif()

    add_subdirectory(scripts)
    add_subdirectory(mk4_module)

endif (PY_EXE)
