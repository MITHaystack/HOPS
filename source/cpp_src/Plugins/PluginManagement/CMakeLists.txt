hops_internal_include_directories()

#list header files #############################################################
set (PLUGIN_MANAGEMENT_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PluginVisitorFactory.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_FringePlotVisitorFactory.hh
)

#list source files #############################################################
set( PLUGIN_MANAGEMENT_SOURCEFILES
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
)

#python/pybind11 libraries
if(HOPS_USE_PYBIND11)
    hops_internal_include_directories(../PythonBindings/Containers/include)
    hops_internal_include_directories(../PythonBindings/Operators/include)
    hops_internal_include_directories(../PythonBindings/Visitors/include)
    list(APPEND PYBIND_APP_LIBS pyMHO_Containers pyMHO_Operators MHO_pyVisitors pybind11::embed)
    add_cflag(USE_PYBIND11)
    list(APPEND PLUGIN_MANAGEMENT_LIBS ${PYBIND_APP_LIBS})
endif(HOPS_USE_PYBIND11)

#julia libraries
if(HOPS_USE_JULIA)
    hops_internal_include_directories(../JuliaBindings/Containers/include)
    hops_internal_include_directories(../JuliaBindings/Operators/include)
    hops_internal_include_directories(../JuliaBindings/Visitors/include)
    list(APPEND JULIA_APP_LIBS jlMHO_Containers jlMHO_Operators MHO_jlVisitors JlCxx::cxxwrap_julia JlCxx::cxxwrap_julia_stl)
    list(APPEND PLUGIN_MANAGEMENT_LIBS ${JULIA_APP_LIBS})
endif(HOPS_USE_JULIA)

#matplot++ library
if(HOPS_USE_MATPLOTPP AND Gnuplot_FOUND)
    hops_internal_include_directories(../../../extern/matplotplusplus/source/)
    hops_internal_include_directories(../MatplotPlugin/include)
    list(APPEND MPP_LIBS MHO_MatplotPlugin matplot)
    add_cflag(USE_MATPLOTPP)
    list(APPEND PLUGIN_MANAGEMENT_LIBS ${MPP_LIBS})
endif()

#compile and install library ###################################################
add_library (MHO_PluginManagement SHARED ${PLUGIN_MANAGEMENT_SOURCEFILES})
target_link_libraries (MHO_PluginManagement ${PLUGIN_MANAGEMENT_LIBS})

hops_install_headers (${PLUGIN_MANAGEMENT_HEADERFILES})
hops_install_libraries (MHO_PluginManagement)

# if (HOPS_ENABLE_TEST)
#     add_subdirectory(test)
# endif (HOPS_ENABLE_TEST)
