#declare header paths ##########################################################
hops_internal_include_directories()

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators pyMHO_Containers pybind11::embed)

#all the test exectuables
set(SOURCE_BASENAMES
    TestPythonContainers
    TestPythonVisibilityAccess
    TestPybind11JSONInterface
    TestPythonParameterStoreInterface
    TestPythonContainerStoreInterface
)

#the actual tests we want to run
set(TEST_BASENAMES
    TestPythonContainers
    #TestPythonVisibilityAccess
    TestPybind11JSONInterface
    TestPythonParameterStoreInterface
    TestPythonContainerStoreInterface
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_compile_options( ${BASENAME} PRIVATE -fvisibility=hidden)
    target_link_libraries( ${BASENAME} PRIVATE ${HOPS_TEST_LIBS})
endforeach( BASENAME )


foreach( BASENAME ${TEST_BASENAMES} )
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )


#temp work around until we do this properly
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mho_test.py" DESTINATION ${PLUGINS_INSTALL_DIR})
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mho_param_test.py" DESTINATION ${PLUGINS_INSTALL_DIR})
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mho_container_store_test.py" DESTINATION ${PLUGINS_INSTALL_DIR})
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mho_test_access.py" DESTINATION ${PLUGINS_INSTALL_DIR})

hops_install_executables( ${SOURCE_BASENAMES} )
