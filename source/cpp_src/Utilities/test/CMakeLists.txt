include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Math/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${EXTERN_HLIBS}/date/include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities)

set(SOURCE_BASENAMES
    TestTimeStampConverter
    TestUUIDGenerator
    TestDirectoryInterface
    TestLegacyRootCodeGenerator
    TestTemplateTypenameDeduction
    TestStationCodeMap
    TestTokenizer
    TestWriteAppend
    TestEncodeDecode
    TestInterval
)

if(HOPS_ENABLE_UNIT_TESTS)
    list(APPEND SOURCE_BASENAMES
        TestStationIdentifier
        TestMD5HashGenerator
        TestExtensibleElement
        TestParameterStore
        TestSerializable
        TestLockFileHandler
        #only includes MHO_LegacyDateConverter.hh; date-tz (if used) comes via MHO_Utilities
        TestLegacyDateConverter
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

#Leap-second / date-library tests: these include date/tz.h directly to exercise
#the upstream backend (TestLeapSecondDrift cross-checks the internal leap-second
#table against the installed system tz db), so they ALWAYS link date-tz regardless
#of HOPS_USE_INTERNAL_LEAP_SECONDS. date-tz is always built (BUILD_TZ_LIB).
set(DATE_TEST_BASENAMES
    TestDateLibraryUsage
    TestLeapSecondDrift
)
foreach( BASENAME ${DATE_TEST_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} MHO_Message MHO_Utilities date-tz)
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )
hops_install_test_executables( ${DATE_TEST_BASENAMES} )

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
