set(MK4UTIL_RELPATH "../../../c_src/mk4util/include")
# set(VEX_RELPATH "../../../c_src/vex/include")
set(MSG_RELPATH "../../../c_src/msg/include")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MK4UTIL_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MSG_RELPATH})

include_directories(${EXTERN_HLIBS}/json/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS
    mk4util
    # vex
    vex2json
)

set(SOURCE_BASENAMES
    # TestParseVex
    TestJson
    # TestJson2
    TestVexParser
    TestJson2Vex
    TestExtractFreqTable
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
