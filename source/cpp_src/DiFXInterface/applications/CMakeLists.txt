hops_internal_include_directories()
hops_internal_include_directories(../../../../extern/cli11)

set(HOPS_DIFXAPP_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Operators MHO_Calibration MHO_DiFXInterface)

set(SOURCE_BASENAMES
    difx2hops
    #TestVexStripper
)

# NOTE: difx2hops does not link against difxio. It gets the DiFX .input data as JSON
# by invoking the difxinput2json helper as a subprocess at runtime
foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_DIFXAPP_LIBS} )
    # Install as a TARGET (not a plain file via install(PROGRAMS)) so CMake
    # rewrites the build-tree RPATH to the relocatable $ORIGIN install RPATH (see
    # cmake/HopsReproducible.cmake) and the binary can find the HOPS libraries
    # once it lives in <prefix>/bin.
    install(TARGETS ${BASENAME} RUNTIME DESTINATION ${BIN_INSTALL_RELDIR})
endforeach( BASENAME )

#the statically linked difx .input file -> JSON converter (GPL-licensed)
add_subdirectory(difxinput2json)

if(HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
