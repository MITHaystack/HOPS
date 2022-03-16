option (HOPS_USE_ROOT "Use CERN ROOT libraries to enable plugin tools for plotting/debug/analysis." OFF)

if (HOPS_USE_ROOT)

include(FindROOT)
find_package(ROOT REQUIRED)
hops_external_include_directories(${ROOT_INCLUDE_DIRS})
add_cflag(USE_ROOT)

#list header files #############################################################
set (ROOTPLUGIN_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_RootCanvasManager.hh
)

#list source files #############################################################
set( ROOTPLUGIN_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_RootCanvasManager.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
set(ROOTPLUGIN_LIBS MHO_Message MHO_Containers MHO_Utilities ${ROOT_LIBRARIES} )

add_library (MHO_RootPlugin SHARED ${ROOTPLUGIN_SOURCEFILES})
target_link_libraries (MHO_RootPlugin ${ROOTPLUGIN_LIBS})

hops_install_headers (${ROOTPLUGIN_HEADERFILES})
hops_install_libraries (MHO_RootPlugin )

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)

endif (HOPS_USE_ROOT)
