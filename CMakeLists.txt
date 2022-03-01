#cmake_minimum_required(VERSION 3.0)
cmake_minimum_required(VERSION 2.8)

set(HOPS_VERSION_NUMBER "4.00")

if(${CMAKE_VERSION} VERSION_GREATER "3.11.0")
cmake_policy(SET CMP0074 OLD)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.0.0")
cmake_policy(SET CMP0048 NEW)
endif()

project(Hops VERSION ${HOPS_VERSION_NUMBER})

# project version

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
include(HopsDefaults)
include(CMakeDependentOption)
include(FindGFORTRAN)
include(FindPGPLOT)
include(UseLATEX)

message(STATUS "Architecture is ${CMAKE_SYSTEM_PROCESSOR}")
set( DEFAULT_INSTALL_DIR  "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_SYSTEM_PROCESSOR}-${HOPS_VERSION_NUMBER}" )
get_filename_component(ABS_DEFAULT_INSTALL_DIR ${DEFAULT_INSTALL_DIR} ABSOLUTE)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${ABS_DEFAULT_INSTALL_DIR}" CACHE PATH "default install path" FORCE )
endif()

#define install directories
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/config")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/doc")
set(LOG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/log")
set(OPENCL_KERNEL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/cl_kernel")
set(SHARE_TEXT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/text")
set(SHARE_VHELP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/vhelp")


file(MAKE_DIRECTORY ${INCLUDE_INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_INSTALL_DIR})
file(MAKE_DIRECTORY ${BIN_INSTALL_DIR})
file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
file(MAKE_DIRECTORY ${DATA_INSTALL_DIR})
file(MAKE_DIRECTORY ${DOC_INSTALL_DIR})
file(MAKE_DIRECTORY ${LOG_INSTALL_DIR})
file(MAKE_DIRECTORY ${OPENCL_KERNEL_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_TEXT_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_VHELP_INSTALL_DIR})

hops_module_paths(Hops)

#compiler flags
hops_require_cpp11()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

#someday we will reduce the number of warnings down to a point where 
#we can turn on -Wall -Wextra, but not today...
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )


#convenience variables
set(CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
set(CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
set(SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
set(PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)
set(EXTERN_HLIBS ${CMAKE_CURRENT_SOURCE_DIR}/extern)



message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION}")

#options for triggering additional dependencies
option(HOPS3_USE_CXX "Build HOPS3 libraries using the C++ compiler." OFF)
if(HOPS3_USE_CXX)
    hops_add_cflag(HOPS3_USE_CXX)
    hops_add_cxxflag(HOPS3_USE_CXX)
endif()

#link against difxio, and build difx2hops 
option(HOPS_USE_DIFXIO "Link against difxio library to read DiFX files" OFF)
if(HOPS_USE_DIFXIO)
    find_package(PkgConfig) # we use pkg-config to 
    pkg_search_module(DIFXIO REQUIRED difxio)
endif()


option( HOPS_ENABLE_TEST "Build developer tests." ON)
if(HOPS_ENABLE_TEST)
    enable_testing()
endif(HOPS_ENABLE_TEST)

option( HOPS_USE_ROOT "Build ROOT based utilities." OFF)

option(HOPS_DEV_USE_OLD "Allow new C++ code to use old hops/fourfit C-libraries, beyond what is absolutely necessary." OFF)


#python package setup
find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)

if (PYTHON_EXECUTABLE)
  #figure out the python install path
  #so we can add it to PYTHONPATH
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pypath.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
  )
  execute_process(COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/pypath.py"  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE PYTHON_SITE_PREFIX)
  message(STATUS "Python path = ${PYTHON_SITE_PREFIX}")

string(STRIP "${PYTHON_SITE_PREFIX}" PYTHON_SITE_PREFIX)

set( PYTHON_MODULE_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_PREFIX}")

#now add the python source dir
add_subdirectory(${PYSOURCE})

endif (PYTHON_EXECUTABLE)

option(HOPS_USE_PYBIND11 "Build python bindings with pybind11" OFF)
if(HOPS_USE_PYBIND11)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11)
endif(HOPS_USE_PYBIND11)


#add an option on whether or not to use FFTW3
option (HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms" OFF)
if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    hops_external_include_directories (${FFTW_INCLUDE_DIRS})
    hops_add_cxxflag(HOPS_USE_FFTW3)
endif (HOPS_USE_FFTW3)




################################################################################
add_subdirectory(${CSOURCE})
add_subdirectory(${EXTERN_HLIBS}/json)
add_subdirectory(${CPPSOURCE})

add_subdirectory(doc)


#configure environment set-up script
configure_file( ${SHSOURCE}/hopsenv.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops.bash @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hops.bash DESTINATION ${BIN_INSTALL_DIR} )
