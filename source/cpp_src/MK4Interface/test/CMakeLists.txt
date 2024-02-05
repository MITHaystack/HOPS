
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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Containers MHO_MK4Interface mk4util dfio vex )

set(SOURCE_BASENAMES
    TestMK4VexInterface
    TestMK4Import
    # TestMK4FringeExport
    TestMK4StationImport
    TestMK4Type200Convert
    TestMK4Type201Convert
    TestMK4Type202Convert
   # TestMK4Type203Convert
    TestMK4Type204Convert
    TestMK4Type205Convert
   # TestMK4Type206Convert
   # TestMK4Type207Convert
    TestMK4Type208Convert
   # TestMK4Type210Convert
   # TestMK4Type212Convert
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
