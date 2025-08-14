hops_internal_include_directories()
hops_internal_include_directories(../../../../extern/matplotplusplus/source/)

#list header files #############################################################
set (MATPLOTPLUGIN_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PlotDataExtractor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_BasicPlotVisitor.hh
)

#list source files #############################################################
set( MATPLOTPLUGIN_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PlotDataExtractor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_BasicPlotVisitor.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#list of c++ libraries
set(MATPLOTPLUGIN_LIBS MHO_Message MHO_Math MHO_Utilities MHO_Operators MHO_Containers matplot)

#compile and install library ###################################################
add_library (MHO_MatplotPlugin SHARED ${MATPLOTPLUGIN_SOURCEFILES})
target_link_libraries (MHO_MatplotPlugin ${MATPLOTPLUGIN_LIBS})

hops_install_headers (${MATPLOTPLUGIN_HEADERFILES})
hops_install_libraries (MHO_MatplotPlugin )

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
