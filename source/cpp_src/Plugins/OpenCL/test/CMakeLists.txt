hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Math/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Operators/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Math/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Containers/include)
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Operators/include)

set(HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_OpenCLPlugin)

set(SOURCE_BASENAMES
    TestOpenCLInterface
    TestOpenCLNDArrayBuffer
    #TestOpenCLMultidimensionalFastFourierTransform
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
