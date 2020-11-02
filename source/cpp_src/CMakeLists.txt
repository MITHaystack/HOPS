#add an option to turn on/off debug messages, off by default
option(ENABLE_DEBUG_MSG "Enable passing of debug messages" OFF)
if(ENABLE_DEBUG_MSG)
    hops_add_cxxflag(HOPS_ENABLE_DEBUG_MSG)
endif(ENABLE_DEBUG_MSG)

#source sub directories (in order of dependence...this is important!)
add_subdirectory(${CPPSOURCE}/Message)
add_subdirectory(${CPPSOURCE}/Containers)
add_subdirectory(${CPPSOURCE}/Utilities)
add_subdirectory(${CPPSOURCE}/Math)
add_subdirectory(${CPPSOURCE}/MK4Interface)
add_subdirectory(${CPPSOURCE}/Applications)
# add_subdirectory(${CPPSOURCE}/Bindings)
