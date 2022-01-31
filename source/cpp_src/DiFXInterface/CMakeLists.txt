#list header files #############################################################
set (DIFXINTERFACE_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DiFXInputInterface.hh
)

#list source files #############################################################
set( DIFXINTERFACE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DiFXInputInterface.cc
)

#declare header paths ##########################################################
set(VEX_RELPATH "../../c_src/vex/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${VEX_RELPATH})

hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################

set(DIFXINTERFACE_LIBS vex vex2json MHO_Message MHO_Utilities)

# we use pkg-config to find difxio
find_package(PkgConfig) 
pkg_search_module(DIFXIO REQUIRED difxio)
hops_external_include_directories (${DIFXIO_INCLUDE_DIRS})

add_library (MHO_DiFXInterface SHARED ${DIFXINTERFACE_SOURCEFILES})
target_link_libraries (MHO_DiFXInterface ${DIFXINTERFACE_LIBS} ${DIFXIO_LDFLAGS})

hops_install_headers (${DIFXINTERFACE_HEADERFILES})
hops_install_libraries (MHO_DiFXInterface)

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)


# list(APPEND DIFXIO_DEP_SOURCE_BASENAMES
#     difx2hops
# )
# 
# foreach( BASENAME ${DIFXIO_DEP_SOURCE_BASENAMES} )
#     add_executable( ${BASENAME} ${BASENAME}.cc )
#     target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS} ${DIFXIO_LDFLAGS})
# endforeach( BASENAME )

hops_install_executables( ${DIFXIO_DEP_SOURCE_BASENAMES} )

