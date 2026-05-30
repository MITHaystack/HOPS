include_directories(${EXTERN_HLIBS}/json/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS
    MHO_Message
    MHO_Utilities
    MHO_Vex2JSON
)

set(SOURCE_BASENAMES
    TestJson
    TestJson2
    TestVexParser
    TestJson2Vex
    TestExtractFreqTable
)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestVexGenerator
        TestVexBlockParser
        TestVexTokenProcessor
        TestVexParameter
        TestVexLine
        TestVexElementLineGenerator
        TestVexParserUnit
    )
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

if(HOPS_ENABLE_UNIT_TESTS)
    target_compile_definitions(TestVexGenerator
        PRIVATE VEX_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
                VEX_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/test_output"
    )
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_output)
    add_test(NAME TestVexGenerator_test COMMAND TestVexGenerator)

    target_compile_definitions(TestVexBlockParser
        PRIVATE VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexBlockParser_test COMMAND TestVexBlockParser)

    target_compile_definitions(TestVexTokenProcessor
        PRIVATE VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexTokenProcessor_test COMMAND TestVexTokenProcessor)

    target_compile_definitions(TestVexParameter
        PRIVATE VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexParameter_test COMMAND TestVexParameter)

    target_compile_definitions(TestVexLine
        PRIVATE VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexLine_test COMMAND TestVexLine)

    target_compile_definitions(TestVexElementLineGenerator
        PRIVATE VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexElementLineGenerator_test COMMAND TestVexElementLineGenerator)

    target_compile_definitions(TestVexParserUnit
        PRIVATE VEX_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
                VEX_FORMAT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../format"
    )
    add_test(NAME TestVexParserUnit_test COMMAND TestVexParserUnit)
endif()

hops_install_test_executables( ${SOURCE_BASENAMES} )

#testing scripts


set(TEST_SCRIPT_BASENAMES chk_v2j chk_v2j_wh2)

foreach( BASENAME ${TEST_SCRIPT_BASENAMES} )
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.sh ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh @ONLY)
    add_test(NAME ${BASENAME}_test COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.sh )
    set_tests_properties(${BASENAME}_test PROPERTIES DEPENDS DIFX_TESTDATA_UNPACK)
    set_tests_properties(${BASENAME}_test PROPERTIES ENVIRONMENT "srcdir=${HOPS_TEST_DATA_DIR}")
endforeach( BASENAME )
