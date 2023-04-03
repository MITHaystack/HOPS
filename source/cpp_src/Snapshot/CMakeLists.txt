#list header files #############################################################
set (SNAPSHOT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Snapshot.hh
)

#list source files #############################################################

set( SNAPSHOT_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Snapshot.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(SNAPSHOT_LIBS MHO_Message MHO_Utilities MHO_Containers)

# add_compile_definitions()
add_compile_definitions("HOPS_SNAPSHOT_DIR=\"${SNAPSHOT_INSTALL_DIR}\"")

add_library (MHO_Snapshot SHARED ${SNAPSHOT_SOURCEFILES})
target_link_libraries (MHO_Snapshot ${SNAPSHOT_LIBS})

hops_install_headers (${SNAPSHOT_HEADERFILES})
hops_install_libraries (MHO_Snapshot )

if (HOPS_ENABLE_TEST)
   add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
