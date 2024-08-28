#declare header paths ##########################################################
hops_internal_include_directories()

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators pybind11::embed pyMHO_Containers pyMHO_Operators)

set(SOURCE_BASENAMES
    # TestPythonOperatorInheritance
    TestPythonGenericOperator
    #TestPythonUnaryOperator
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_compile_options( ${BASENAME} PRIVATE -fvisibility=hidden)
    target_link_libraries( ${BASENAME} PRIVATE ${HOPS_TEST_LIBS})
    #add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

#temp work around until we do this properly
install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/mho_operators.py" DESTINATION ${PLUGINS_INSTALL_DIR})

hops_install_executables( ${SOURCE_BASENAMES} )
