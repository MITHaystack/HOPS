hops_internal_include_directories()

set (HOPS_TEST_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Snapshot)

set(SOURCE_BASENAMES
    TestSnapshot
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
    add_test(NAME ${BASENAME}_test COMMAND ${BASENAME})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
