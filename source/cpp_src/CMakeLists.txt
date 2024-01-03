#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)
if(ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

option(ENABLE_SNAPSHOTS "Enable object dumps to snapshot file for debugging/inspection" OFF)
if(ENABLE_SNAPSHOTS)
    hops_add_cxxflag(HOPS_ENABLE_SNAPSHOTS)
    add_compile_definitions("HOPS_SNAPSHOT_DIR=${SNAPSHOT_INSTALL_DIR}")
endif(ENABLE_SNAPSHOTS)

#add option to throw errors on bad return values
option(ENABLE_STEPWISE_CHECK "When specified, check return value of operation initialization/execution, throw on error" OFF)
if(ENABLE_STEPWISE_CHECK)
    hops_add_cxxflag(HOPS_ENABLE_STEPWISE_CHECK)
endif(ENABLE_STEPWISE_CHECK)

hops_require_cpp11()

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
add_subdirectory(${CPPSOURCE}/Fringe) #library of fringe-fitting related functionality
add_subdirectory(${CPPSOURCE}/MK4Interface) #interface to MK4 data types and import


# if(HOPS_USE_DIFXIO)
# add_subdirectory(${CPPSOURCE}/DiFXInterface)
# endif(HOPS_USE_DIFXIO)
#
# add_subdirectory(${CPPSOURCE}/Plugins/OpenCL) #opencl plugin
#
# if(HOPS_USE_PYBIND11)
#     add_subdirectory(${CPPSOURCE}/Plugins/PythonBindings) #python bindings
# endif(HOPS_USE_PYBIND11)
#
# add_subdirectory(${CPPSOURCE}/Plugins/Root) #CERN ROOT plugins
#
# add_subdirectory(${CPPSOURCE}/Applications) #applications
