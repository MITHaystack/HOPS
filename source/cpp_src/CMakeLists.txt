#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" ON)
if(ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

option(ENABLE_EXTRA_VERBOSE_MSG "When enabled adds additional file:line origin information to all messages" OFF)
if(ENABLE_EXTRA_VERBOSE_MSG)
    hops_add_cxxflag(HOPS_EXTRA_VERBOSE_MSG)
endif(ENABLE_EXTRA_VERBOSE_MSG)

option(ENABLE_COLOR_MSG "Enable color messages." ON)
if(ENABLE_COLOR_MSG)
    hops_add_cxxflag(HOPS_COLOR_MSG)
endif(ENABLE_COLOR_MSG)

option(ENABLE_SNAPSHOTS "Enable object dumps to snapshot file for debugging/inspection" OFF)
if(ENABLE_SNAPSHOTS)
    hops_add_cxxflag(HOPS_ENABLE_SNAPSHOTS)
    add_compile_definitions("HOPS_SNAPSHOT_DIR=${SNAPSHOT_INSTALL_DIR}")
endif(ENABLE_SNAPSHOTS)

#add option to throw errors on bad return values
option(ENABLE_STEPWISE_CHECK "When specified, checks the return value of operator initialization/execution, throws on error" OFF)
if(ENABLE_STEPWISE_CHECK)
    hops_add_cxxflag(HOPS_ENABLE_STEPWISE_CHECK)
endif(ENABLE_STEPWISE_CHECK)

option(HOPS_ENABLE_DEV_TODO "Enable developer to-do/fix-me compiler messages" OFF)
if(HOPS_ENABLE_DEV_TODO)
    hops_add_cxxflag(HOPS_ENABLE_DEV_TODO)
endif()


#add the plugin directory definition, so we can search in this path
add_compile_definitions("HOPS_DEFAULT_PLUGINS_DIR=${PLUGINS_INSTALL_DIR}")

#source sub directories (in order of dependence...this is important!)
add_subdirectory(${CPPSOURCE}/Message) #the messaging library
add_subdirectory(${CPPSOURCE}/Utilities) #other generic utilities
add_subdirectory(${CPPSOURCE}/Vex2JSON) #library to convert ovex to json
add_subdirectory(${CPPSOURCE}/Math) #the generic math library
add_subdirectory(${CPPSOURCE}/Containers) #data container base classes and generic containers
add_subdirectory(${CPPSOURCE}/Operators) #data operator base classes and generic operators

add_subdirectory(${CPPSOURCE}/Calibration) #VLBI specific data containers and operators
add_subdirectory(${CPPSOURCE}/Control) #library to parse old-fourfit control files
add_subdirectory(${CPPSOURCE}/Initialization) #library to build objects/operators

add_subdirectory(${CPPSOURCE}/MK4Interface) #interface to MK4 data types and import


if(HOPS_USE_DIFXIO)
add_subdirectory(${CPPSOURCE}/DiFXInterface) #difxio input support
endif(HOPS_USE_DIFXIO)

if(HOPS_USE_PYBIND11)
    add_subdirectory(${CPPSOURCE}/Plugins/PythonBindings) #python bindings
endif(HOPS_USE_PYBIND11)

if(HOPS_USE_OPENCL)
add_subdirectory(${CPPSOURCE}/Plugins/OpenCL) #opencl plugin
endif(HOPS_USE_OPENCL)

if(HOPS_USE_CUDA)
add_subdirectory(${CPPSOURCE}/Plugins/CUDA) #cuda plugin
endif(HOPS_USE_CUDA)

add_subdirectory(${CPPSOURCE}/Fringe) #library of fringe-fitting related functionality

add_subdirectory(${CPPSOURCE}/Applications) #applications
