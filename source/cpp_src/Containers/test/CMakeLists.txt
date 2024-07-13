include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Operators/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Operators MHO_Containers)

set(SOURCE_BASENAMES
    TestArrayWrapper
    TestVectorContainer
    TestScalarContainer
    TestAxis
    TestAxisPack
    TestTableContainer
    TestContainerNames
    TestContainerStore
    TestTableContainerAccess
    #TestTableContainerMultiplication
    TestSelectRepack
    TestIntervalLabelTree
    TestObjectTags
    TestSnapshot
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )


hops_install_executables( ${SOURCE_BASENAMES} )
