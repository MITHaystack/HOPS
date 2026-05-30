include_directories(${EXTERN_HLIBS}/json/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS
    MHO_Message MHO_Utilities MHO_Control
)

set(SOURCE_BASENAMES )

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestControlFileParser
        TestControlConditionEvaluator
        TestControlElementParser
        TestControlTokenProcessor
        TestControlDefinitions
    )
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )
