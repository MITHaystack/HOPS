include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
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
    TestParameterStore
)

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
