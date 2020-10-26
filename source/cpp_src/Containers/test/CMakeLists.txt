include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS MHOMessage MHOUtilities)

set(SOURCE_BASENAMES
    TestArrayWrapper
    TestVectorContainer
    TestScalarContainer
    TestTensorContainer
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
