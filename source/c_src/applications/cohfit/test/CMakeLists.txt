# Build & register the in-tree GSL example programs as ctest cases.
#
# Each example is built as a non-installed test fixture, then two tests
# are registered per example:
#   <name>_run_test    -- runs the binary, redirecting output to <name>.log
#   <name>_check_test  -- runs the shell check script against the .log
# The check test DEPENDS on the run test so ctest orders them correctly.

find_program(BASH_PROGRAM bash REQUIRED)
if(NOT BASH_PROGRAM)
    return()
endif()

include_directories(${GSL_INCLUDE_DIRS})

set(TEST_SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})

# Helper: build an example exe (linked against GSL+m), drop it in the
# current binary dir, but don't install it.
macro(_cohfit_add_example name)
    add_executable(${name} EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/${name}.c)
    target_link_libraries(${name} ${GSL_LIBRARIES} m)
    # Make sure 'make test' (which runs ctest) will trigger a build of the
    # example via add_test() -> CMake provides the build-and-test fixture below.
    add_test(NAME ${name}_build
             COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
                                     --target ${name} --config $<CONFIG>)
    set_tests_properties(${name}_build PROPERTIES FIXTURES_SETUP ${name}_fixture)
endmacro()

# Helper: register the run + check pair for an example. 'log' is the
# expected log filename (the check script greps it); 'check_script' is
# the .sh;
macro(_cohfit_add_example_tests name log check_script)
    set(check_arg "${ARGN}")
    add_test(NAME ${name}_run_test
             COMMAND ${BASH_PROGRAM} -c
             "$<TARGET_FILE:${name}> > ${log} 2>&1")
    set_tests_properties(${name}_run_test PROPERTIES
        FIXTURES_REQUIRED ${name}_fixture
        FIXTURES_SETUP ${name}_ran)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${check_script}
                   ${CMAKE_CURRENT_BINARY_DIR}/${check_script} @ONLY)
    add_test(NAME ${name}_check_test
             COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${check_script}
                                     ${check_arg})
    set_tests_properties(${name}_check_test PROPERTIES
        FIXTURES_REQUIRED ${name}_ran
        ENVIRONMENT "srcdir=${TEST_SRCDIR}"
        SKIP_RETURN_CODE 77)
endmacro()

# exminimizer test
_cohfit_add_example(exminimizer)
_cohfit_add_example_tests(exminimizer exminimizer.log exminimizer.sh)

# test expfitexample 
_cohfit_add_example(expfitexample)
_cohfit_add_example_tests(expfitexample expfitexample.log expfitcheck.sh)

# test cbsplinelsqex: two variants share a check script; pick which to build
# based on the GSL version available.
if(GSL_VERSION VERSION_GREATER_EQUAL "2.8")
    _cohfit_add_example(cbsplinelsqex)
    _cohfit_add_example_tests(cbsplinelsqex cbsplinelsqex2p8.log cbsplinecheck.sh 2p8)
else()
    _cohfit_add_example(cbsplinelsqex2p7)
    _cohfit_add_example_tests(cbsplinelsqex2p7 cbsplinelsqex2p7.log cbsplinecheck.sh 2p7)
endif()
