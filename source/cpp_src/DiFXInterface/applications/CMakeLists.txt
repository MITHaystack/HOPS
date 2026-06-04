hops_internal_include_directories()
hops_internal_include_directories(../../../../extern/cli11)

set(HOPS_DIFXAPP_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Operators MHO_Calibration MHO_DiFXInterface)

set(SOURCE_BASENAMES
    difx2hops
    #TestVexStripper
)

find_package(PkgConfig) # we use pkg-config to find difxio
pkg_search_module(DIFXIO REQUIRED difxio)

hops_external_include_directories (${DIFXIO_INCLUDE_DIRS})

list(APPEND DIFXIO_DEP_SOURCE_BASENAMES
    difx2hops
)

foreach( BASENAME ${DIFXIO_DEP_SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_DIFXAPP_LIBS} PkgConfig::DIFXIO)
    # Install as a TARGET (not a plain file via install(PROGRAMS)) so CMake
    # rewrites the build-tree RPATH to the relocatable $ORIGIN install RPATH (see
    # cmake/HopsReproducible.cmake) and the binary can find the HOPS libraries
    # once it lives in <prefix>/bin. INSTALL_RPATH_USE_LINK_PATH additionally
    # appends the external difxio library dir to the install RPATH so
    # libdifxio.so (which lives outside the HOPS install tree) can still found.
    set_target_properties( ${BASENAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH ON )
    install(TARGETS ${BASENAME} RUNTIME DESTINATION ${BIN_INSTALL_RELDIR})
endforeach( BASENAME )


if(HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
