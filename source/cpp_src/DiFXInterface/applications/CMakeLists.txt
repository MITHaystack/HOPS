hops_internal_include_directories()

set(HOPS_DIFXAPP_LIBS MHO_Message MHO_Utilities MHO_Math MHO_Operators MHO_Calibration MHO_DiFXInterface)

set(SOURCE_BASENAMES
    difx2hops
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
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME} DESTINATION ${BIN_INSTALL_DIR})
endforeach( BASENAME )
