include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Math/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${EXTERN_HLIBS}/date/include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities date-tz)

set(SOURCE_BASENAMES
    TestTimeStampConverter
    TestUUIDGenerator
    TestDirectoryInterface
    TestLegacyRootCodeGenerator
    TestTemplateTypenameDeduction
    TestStationCodeMap
    TestDateLibraryUsage
    TestTokenizer
    TestWriteAppend
    TestEncodeDecode
    TestInterval
)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestLegacyDateConverter
        TestStationIdentifier
        TestMD5HashGenerator
        TestExtensibleElement
        TestParameterStore
        TestSerializable
        TestLockFileHandler
    )
endif()

if(HOPS_USE_MPI)
    list (APPEND SOURCE_BASENAMES
        TestMPIInterface
    )
    list(APPEND HOPS_TEST_LIBS MPI::MPI_CXX)
endif(HOPS_USE_MPI)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )

if(HOPS_ENABLE_UNIT_TESTS)
    #multi-process write-lock contention stress test: a helper worker driven by a
    #bash orchestrator that launches many copies in parallel and checks that the
    #assigned fringe sequence numbers are unique and contiguous.
    add_executable( LockContentionWorker LockContentionWorker.cc )
    target_link_libraries( LockContentionWorker ${HOPS_TEST_LIBS})

    find_program(BASH_PROGRAM bash)
    if(BASH_PROGRAM)
        add_test(NAME LockContention_test
                 COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/run_lock_contention.sh
                         $<TARGET_FILE:LockContentionWorker>)
        set_tests_properties(LockContention_test PROPERTIES TIMEOUT 120)
    else()
        message(STATUS "bash not found; skipping LockContention_test")
    endif()
endif()
