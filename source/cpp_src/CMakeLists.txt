#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)
if(ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

hops_require_cpp11()

#source sub directories (in order of dependence...this is important!)
add_subdirectory(${CPPSOURCE}/Message) #the messaging library
add_subdirectory(${CPPSOURCE}/Math) #the generic math library
add_subdirectory(${CPPSOURCE}/Utilities) #other generic utilities
add_subdirectory(${CPPSOURCE}/Containers) #data container base classes and generic containers
add_subdirectory(${CPPSOURCE}/Operators) #data operator base classes and generic operators
add_subdirectory(${CPPSOURCE}/Calibration) #VLBI specific data containers and operators
add_subdirectory(${CPPSOURCE}/vex2json) #library to convert ovex to json
add_subdirectory(${CPPSOURCE}/MK4Interface) #interface to MK4 data types and import
if(HOPS_USE_DIFXIO)
add_subdirectory(${CPPSOURCE}/DiFXInterface)
endif(HOPS_USE_DIFXIO)

add_subdirectory(${CPPSOURCE}/Plugins/OpenCL) #opencl plugin

if(HOPS_USE_PYBIND11)
    add_subdirectory(${CPPSOURCE}/Plugins/PythonBindings) #python bindings
endif(HOPS_USE_PYBIND11)

add_subdirectory(${CPPSOURCE}/Applications) #applications

#transitory applications for migration
add_subdirectory(${CPPSOURCE}/Applications/fourfit)
