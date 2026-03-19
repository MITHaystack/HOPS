#add an option to turn on/off debug messages, off by default
option(HOPS_ENABLE_DEBUG_MSG "Enable passing of debug messages" ON)
if(HOPS_ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(HOPS_ENABLE_DEBUG_MSG)

option(HOPS_ENABLE_EXTRA_VERBOSE_MSG "When enabled adds additional file:line origin information to all messages" OFF)
if(HOPS_ENABLE_EXTRA_VERBOSE_MSG)
    hops_add_cxxflag(HOPS_EXTRA_VERBOSE_MSG)
endif(HOPS_ENABLE_EXTRA_VERBOSE_MSG)

option(HOPS_ENABLE_COLOR_MSG "Enable color messages." ON)
if(HOPS_ENABLE_COLOR_MSG)
    hops_add_cxxflag(HOPS_COLOR_MSG)
endif(HOPS_ENABLE_COLOR_MSG)

option(HOPS_ENABLE_SNAPSHOTS "Enable object dumps to snapshot file for debugging/inspection" OFF)
if(HOPS_ENABLE_SNAPSHOTS)
    hops_add_cxxflag(HOPS_ENABLE_SNAPSHOTS)
    add_compile_definitions("HOPS_SNAPSHOT_DIR=${SNAPSHOT_INSTALL_DIR}")
endif(HOPS_ENABLE_SNAPSHOTS)

#add option to throw errors on bad return values
option(HOPS_ENABLE_STEPWISE_CHECK "When specified, checks the return value of operator initialization/execution, throws on error" OFF)
if(HOPS_ENABLE_STEPWISE_CHECK)
    hops_add_cxxflag(HOPS_ENABLE_STEPWISE_CHECK)
endif(HOPS_ENABLE_STEPWISE_CHECK)

option(HOPS_ENABLE_PROFILER "Enable the built-in profiler tool for the HOPS4 code" OFF)
if(HOPS_ENABLE_PROFILER)
    hops_add_cxxflag(HOPS_USE_PROFILER)
endif(HOPS_ENABLE_PROFILER)

option(HOPS_ENABLE_DEV_TODO "Enable developer to-do/fix-me compiler messages" OFF)
if(HOPS_ENABLE_DEV_TODO)
    hops_add_cxxflag(HOPS_ENABLE_DEV_TODO)
endif()

#add the plugin directory definition, so we can search in this path
add_compile_definitions("HOPS_DEFAULT_PLUGINS_DIR=${PLUGINS_INSTALL_DIR}")

#configure the version header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hops_version.hh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops_version.hh)
hops_install_headers(${CMAKE_CURRENT_BINARY_DIR}/hops_version.hh)
hops_include_directories(${CMAKE_CURRENT_BINARY_DIR})

#########################################################################
# Core HOPS4 source sub directories 
# listed in order of dependence...this is important!
add_subdirectory(${CPPSOURCE}/Message) #the messaging library
add_subdirectory(${CPPSOURCE}/Utilities) #other generic utilities
add_subdirectory(${CPPSOURCE}/Vex2JSON) #library to convert ovex to json
add_subdirectory(${CPPSOURCE}/Math) #the generic math library
add_subdirectory(${CPPSOURCE}/Containers) #data container base classes and generic containers
add_subdirectory(${CPPSOURCE}/Operators) #data operator base classes and generic operators
add_subdirectory(${CPPSOURCE}/Calibration) #VLBI specific data containers and operators
add_subdirectory(${CPPSOURCE}/Control) #library to parse fourfit control files
add_subdirectory(${CPPSOURCE}/Initialization) #library to build objects/operators
add_subdirectory(${CPPSOURCE}/MK4Interface) #interface to MK4 data types and import


#######################################################################
## Accelerator plugins (these are generally ifdef'd into the code which uses them)

if(HOPS_USE_OPENMP)
    #OpenMP (cpu parallelism) plugin
    add_subdirectory(${CPPSOURCE}/Plugins/OpenMP)
endif(HOPS_USE_OPENMP)

if(HOPS_USE_CUDA)
    #cuda plugin (GPU parallelism for FFTs)
    add_subdirectory(${CPPSOURCE}/Plugins/CUDA)
endif(HOPS_USE_CUDA)

if(HOPS_USE_OPENCL)
    #opencl plugin (GPU/accelerator parallelism)
    add_subdirectory(${CPPSOURCE}/Plugins/OpenCL) 
endif(HOPS_USE_OPENCL)

#########################################################################
# HOPS4 library of fringe-fitting related functionality 
# this library may depend on any of the above accelerator plugins (if enabled)
add_subdirectory(${CPPSOURCE}/Fringe) 

#########################################################################
# plugins which extend capabilities, and external language interfaces

if(HOPS_USE_DIFXIO)
    #difxio input support
    add_subdirectory(${CPPSOURCE}/DiFXInterface) 
endif(HOPS_USE_DIFXIO)

if(HOPS_USE_PYBIND11)
    #python bindings (container access, and user plugins)
    add_subdirectory(${CPPSOURCE}/Plugins/PythonBindings)
endif(HOPS_USE_PYBIND11)

if(HOPS_USE_JULIA)
    add_subdirectory(${CPPSOURCE}/Plugins/JuliaBindings) #julia bindings
endif(HOPS_USE_JULIA)

if(HOPS_USE_MATPLOTPP AND Gnuplot_FOUND)
    #Matplot++ (gnuplot) wrapper support (fast default fringe plotting)
    add_subdirectory(${CPPSOURCE}/Plugins/MatplotPlugin)
endif()

if(HOPS_USE_HDF5)
    #hdf5 plugin (export to HDF5 files)
    add_subdirectory(${CPPSOURCE}/Plugins/HDF5)
endif(HOPS_USE_HDF5)

#########################################################################
# Aggregate all optional capability plugin deps into a single INTERFACE
# target. Any library or executable that needs them only needs to link to this one
# target (MHO_OptionalPlugins), adding a new plugin requires editing this block, but 
# we don't need to have a bunch of if statements wrapping plugin dependencies
# for executables/libs downstream
add_library(MHO_OptionalPlugins INTERFACE)

if(HOPS_USE_PYBIND11)
    target_link_libraries(MHO_OptionalPlugins INTERFACE
        pyMHO_Containers pyMHO_Operators MHO_pyVisitors pybind11::embed)
    target_include_directories(MHO_OptionalPlugins INTERFACE
        ${CPPSOURCE}/Plugins/PythonBindings/Containers/include
        ${CPPSOURCE}/Plugins/PythonBindings/Operators/include
        ${CPPSOURCE}/Plugins/PythonBindings/Visitors/include)
    target_compile_definitions(MHO_OptionalPlugins INTERFACE USE_PYBIND11)
endif()

if(HOPS_USE_JULIA)
    target_link_libraries(MHO_OptionalPlugins INTERFACE
        jlMHO_Containers jlMHO_Operators MHO_jlVisitors
        JlCxx::cxxwrap_julia JlCxx::cxxwrap_julia_stl)
    target_include_directories(MHO_OptionalPlugins INTERFACE
        ${CPPSOURCE}/Plugins/JuliaBindings/Containers/include
        ${CPPSOURCE}/Plugins/JuliaBindings/Operators/include
        ${CPPSOURCE}/Plugins/JuliaBindings/Visitors/include)
endif()

if(HOPS_USE_MATPLOTPP AND Gnuplot_FOUND)
    target_link_libraries(MHO_OptionalPlugins INTERFACE MHO_MatplotPlugin matplot)
    target_include_directories(MHO_OptionalPlugins INTERFACE
        ${CPPSOURCE}/../extern/matplotplusplus/source/
        ${CPPSOURCE}/Plugins/MatplotPlugin/include)
    target_compile_definitions(MHO_OptionalPlugins INTERFACE USE_MATPLOTPP)
endif()

if(HOPS_USE_HDF5)
    find_package(HDF5 COMPONENTS C CXX HL QUIET)
    if(HDF5_FOUND)
        target_link_libraries(MHO_OptionalPlugins INTERFACE MHO_HDF5Interface)
        target_include_directories(MHO_OptionalPlugins INTERFACE
            ${CPPSOURCE}/Plugins/HDF5/include
            ${HDF5_INCLUDE_DIRS})
        target_compile_definitions(MHO_OptionalPlugins INTERFACE USE_HDF5)
    endif()
endif()

#management layer for optional plugins
add_subdirectory(${CPPSOURCE}/Plugins/PluginManagement)

######################################################################
# lastly we build our applications 
# which may depend on any of the enabled plugins
add_subdirectory(${CPPSOURCE}/Applications) 


