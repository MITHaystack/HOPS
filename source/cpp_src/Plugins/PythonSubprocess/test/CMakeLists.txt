include_directories(SYSTEM ${EXTERN_HLIBS}/json/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(HOPS_TEST_LIBS
    MHO_Message MHO_Utilities MHO_Control MHO_Containers MHO_PySubprocess
)

set(SOURCE_BASENAMES)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestSubprocessPyControlEvaluator
        TestSubprocessPythonPlotVisitor
    )
endif()

foreach(BASENAME ${SOURCE_BASENAMES})
    add_executable(${BASENAME} ${BASENAME}.cc)
    target_link_libraries(${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
    #the control-format JSON and the hops_control python package are resolved via
    #$HOPS_INSTALL (see MHO_PythonSubprocessRunner / MHO_ControlDefinitions); point
    #it at the install tree. The test SKIPs (77) if those are not present, so it
    #still needs 'make install' to actually exercise the subprocess path.
    set_tests_properties(${BASENAME}_test PROPERTIES
        ENVIRONMENT "HOPS_INSTALL=${CMAKE_INSTALL_PREFIX}"
        SKIP_RETURN_CODE 77)
endforeach(BASENAME)

hops_install_test_executables(${SOURCE_BASENAMES})
