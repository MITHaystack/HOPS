include_directories(${EXTERN_HLIBS}/json/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set (HOPS_TEST_LIBS
    MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers MHO_Calibration MHO_Control MHO_Initialization MHO_Fringe
)

set(SOURCE_BASENAMES
    TestAFileFormat
)

#compile and install library ###################################################

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
