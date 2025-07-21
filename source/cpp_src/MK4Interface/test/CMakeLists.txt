
set(MK4UTIL_RELPATH "../../../c_src/mk4util/include")
set(DFIO_RELPATH "../../../c_src/dfio/include")
set(VEX_RELPATH "../../../c_src/vex/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${DFIO_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Message/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Containers/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Utilities/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Math/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Vex2JSON/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Include CLI11 for command line 
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../extern/cli11)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Containers MHO_MK4Interface MHO_Vex2JSON mk4util dfio vex )

set(SOURCE_BASENAMES
    TestMK4VexInterface
    TestMK4Import
    TestHOPS2Corel
    #hops2mark4
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_test_executables( ${SOURCE_BASENAMES} )
