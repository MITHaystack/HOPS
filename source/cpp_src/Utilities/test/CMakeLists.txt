include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${EXTERN_HLIBS}/date/include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities date-tz)

set(SOURCE_BASENAMES
    TestMultiTypeMap
    TestIntervalLabelTree
    TestTimeStampConverter
    TestUUIDGenerator
    TestObjectTags
    TestDirectoryInterface
    TestLegacyRootCodeGenerator
    TestTemplateTypenameDeduction
    TestStationCodeMap
    TestDateLibraryUsage
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
