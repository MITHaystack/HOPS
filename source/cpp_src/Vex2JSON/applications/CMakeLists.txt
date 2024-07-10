hops_internal_include_directories()

set(EXE_LIBS
    MHO_Message
    MHO_Utilities
    MHO_Vex2JSON
)

set(SOURCE_BASENAMES
    vex2json
    json2vex
)

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${EXE_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )
