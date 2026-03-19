hops_internal_include_directories()

#list header files #############################################################
set (PLUGIN_MANAGEMENT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_HopsOutputVisitor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_Mark4OutputVisitor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_OutputVisitorFactory.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PluginVisitorFactory.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringePlotVisitorFactory.hh
)

#list source files #############################################################
set( PLUGIN_MANAGEMENT_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_HopsOutputVisitor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_Mark4OutputVisitor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_OutputVisitorFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PluginVisitorFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_FringePlotVisitorFactory.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(PLUGIN_MANAGEMENT_LIBS 
    MHO_Message 
    MHO_Math 
    MHO_Utilities 
    MHO_Operators 
    MHO_Containers 
    MHO_Calibration 
    MHO_Control 
    MHO_Initialization
    MHO_MK4Interface
)

#compile and install library ###################################################
add_library(MHO_PluginManagement SHARED ${PLUGIN_MANAGEMENT_SOURCEFILES})
target_link_libraries(MHO_PluginManagement PUBLIC ${PLUGIN_MANAGEMENT_LIBS} MHO_OptionalPlugins)


hops_install_headers (${PLUGIN_MANAGEMENT_HEADERFILES})
hops_install_libraries (MHO_PluginManagement)

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
