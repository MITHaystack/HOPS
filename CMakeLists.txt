################################################################################
#CMAKE version and policies
cmake_minimum_required(VERSION 3.1)

if(${CMAKE_VERSION} VERSION_GREATER "3.11.0")
cmake_policy(SET CMP0074 OLD)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.3.0")
cmake_policy(SET CMP0060 NEW)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.0.0")
cmake_policy(SET CMP0048 NEW)
endif()
################################################################################

# project version (stuck with these names due to configure.ac for now)
set(MODULE_VERSION_MAJOR 4)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)
set(HOPS_VERSION_NUMBER "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH}")
project(Hops VERSION ${HOPS_VERSION_NUMBER})

#we store the git hash (6 chars) of the last tagged revision in this file
#this must be updated on each release with before running git-archive:
#git-revision.sh | tee ./tag-rev.last
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/tag-rev.last" GIT_REV_VALUE)
string(STRIP "${GIT_REV_VALUE}" HOPS_LAST_GIT_REV)
message(STATUS "Last tagged revision: ${HOPS_LAST_GIT_REV}")

#default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#includes for package finding
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
include(HopsDefaults)
include(CMakeDependentOption)
include(FindGFORTRAN)
include(FindPGPLOT)

#not ready for these yet
#include(CPack) -- use to create system packages (e.g. .deb/.rpm)
#include(CMakePackageConfigHelpers) -- use to create pkgconfig .pc files for external projects

#install prefix
message(STATUS "Architecture is ${CMAKE_SYSTEM_PROCESSOR}")
set( DEFAULT_INSTALL_DIR  "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_SYSTEM_PROCESSOR}-${HOPS_VERSION_NUMBER}" )
get_filename_component(ABS_DEFAULT_INSTALL_DIR ${DEFAULT_INSTALL_DIR} ABSOLUTE)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${ABS_DEFAULT_INSTALL_DIR}" CACHE PATH "default install path" FORCE )
endif()

#define and create install directories #########################################
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
#on some systems the tzdate library gets put here (lib64)
#so make sure it exists and is added to the library path
set(LIB64_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib64") 
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(TEST_BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin/test")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/config")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/doc")
set(LOG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/log")
set(PLUGINS_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/plugin_scripts") #user and other python-plugin scripts
set(SNAPSHOT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/snapshot")
set(OPENCL_KERNEL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/cl_kernel")
set(SHARE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share")
set(SHARE_TEXT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/text")
set(SHARE_VHELP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/vhelp")

file(MAKE_DIRECTORY ${INCLUDE_INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB64_INSTALL_DIR})
file(MAKE_DIRECTORY ${BIN_INSTALL_DIR})
file(MAKE_DIRECTORY ${TEST_BIN_INSTALL_DIR})
file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
file(MAKE_DIRECTORY ${DATA_INSTALL_DIR})
file(MAKE_DIRECTORY ${DOC_INSTALL_DIR})
file(MAKE_DIRECTORY ${LOG_INSTALL_DIR})
file(MAKE_DIRECTORY ${SNAPSHOT_INSTALL_DIR})
file(MAKE_DIRECTORY ${OPENCL_KERNEL_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_TEXT_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_VHELP_INSTALL_DIR})
file(MAKE_DIRECTORY ${PLUGINS_INSTALL_DIR})

#convenience variables
set(CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
set(CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
set(SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
set(PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)
set(PERLSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/perl_src)
set(EXTERN_HLIBS ${CMAKE_CURRENT_SOURCE_DIR}/extern)


################################################################################
#compiler settings
#use c++11 standard by default
hops_require_cpp11()
#hops_require_cpp17()
#hops_require_cpp20()
message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION}")

option(EXTRA_WARNINGS "Turn on extra compiler warnings." OFF)
if(EXTRA_WARNINGS)
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )
else()
    #someday we will reduce the number of warnings down to a point where
    #we can turn on -Wall -Wextra by default, but not today...
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
endif()

################################################################################
#create build timestamp...note: a consequence of this is that the build will not
#be reproducible, and we will be forced to rebuild more files more often
#This is required for legacy behavior of fringe plots (which report build time).
string(TIMESTAMP BUILD_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
message(STATUS "The build timestamp is ${BUILD_TIME}")

################################################################################

#options for triggering additional dependencies
option(HOPS_ENABLE_TEST "Build developer tests." ON)
option(HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms (optional, but highly recommended)" ON)
option(HOPS_USE_DIFXIO "Link against difxio library to read DiFX files" OFF)
option(HOPS_USE_PYBIND11 "Build python bindings with pybind11" ON)
option(HOPS_USE_MPI "Use MPI (Message Passing Interface) for parallel processing" OFF)
option(HOPS3_PYTHON_EXTRAS "Build python (ctypes) interface to hops3 libraries" ON)
option(HOPS_IS_HOPS4 "Install the HOPS4 version of relevant executables (symlinked to expected names, e.g. fourfit, fplot)" OFF)
option(HOPS_USE_OPENCL "Use OpenCL via C++ wrapper API." OFF)
option(HOPS_USE_CUDA "Build CUDA support." OFF)
option(HOPS_USE_HDF5 "Build HDF5 export support." OFF)

option(HOPS3_USE_CXX "Build HOPS3 libraries using the C++ compiler." OFF)
mark_as_advanced(HOPS3_USE_CXX)

if(HOPS3_USE_CXX)
    hops_add_cflag(HOPS3_USE_CXX)
    hops_add_cxxflag(HOPS3_USE_CXX)
    set(HOPS3_PYTHON_EXTRAS OFF CACHE BOOL "(Disabled when HOPS3_USE_CXX=ON)" FORCE )
endif()

if(HOPS_USE_FFTW3)
    find_package(FFTW REQUIRED)
    if(FFTW_FOUND)
        hops_external_include_directories (${FFTW_INCLUDE_DIRS})
        hops_add_cxxflag(HOPS_USE_FFTW3)
    else(FFTW_FOUND)
        message(STATUS "Use of FFTW3 was requested but not found, disabling HOPS_USE_FFTW3")
        set(HOPS_USE_FFTW3 OFF CACHE BOOL "Use FFTW3 for fast fourier transforms" FORCE)
    endif(FFTW_FOUND)
endif (HOPS_USE_FFTW3)

#link against difxio, and build difx2hops
if(HOPS_USE_DIFXIO)
    find_package(PkgConfig) # we use pkg-config to find difxio
    pkg_search_module(DIFXIO REQUIRED difxio)
endif()

#find MPI devel if needed
if (HOPS_USE_MPI)
    find_package(MPI REQUIRED)
endif(HOPS_USE_MPI)

if(HOPS_USE_CUDA)
    hops_add_cxxflag(HOPS_USE_CUDA)
endif(HOPS_USE_CUDA)


#explicity set the command to for vpal library fringe fitter via env var
if(HOPS_IS_HOPS4)
    set(HOPS_VPAL_FOURFIT "fourfit4")
else()
    set(HOPS_VPAL_FOURFIT "fourfit")
endif(HOPS_IS_HOPS4)

################################################################################
#stuff for controlling testing/test-data location
set(TEST_DATA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_data)
#location where test data should be dowloaded to (if the user runs testdata_download_all.sh)
set(HOPS_CACHED_TEST_DATADIR "${DATA_INSTALL_DIR}/test_data" CACHE PATH "Location used to download/cache HOPS test data.")
#actual location to unpack the test data and do the tests
set(HOPS_TEST_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# #look for an environmental variable pointing to a cache of test data
# set(HOPS_USE_CACHED_TEST_DATA OFF)
# # set(HOPS_NEED_REMOTE_DATA ON)
# if (NOT "${HOPS_CACHED_TEST_DATADIR}" STREQUAL "")
#
#     #mark_as_advanced(HOPS_CACHED_TEST_DATADIR)
#     set(HOPS_USE_CACHED_TEST_DATA ON)
#     # set(HOPS_NEED_REMOTE_DATA OFF)
# endif()

# #check that we can download test data if testing is enabled and there is no cache
# cmake_dependent_option( HOPS_ENABLE_REMOTE_TEST_DATA "Download remote data for developer tests." ${HOPS_NEED_REMOTE_DATA} "HOPS_ENABLE_TEST" OFF)

# if(HOPS_ENABLE_TEST)
#     if(NOT HOPS_ENABLE_REMOTE_TEST_DATA AND NOT HOPS_USE_CACHED_TEST_DATA)
#             #we are not downloading test data (and no local cache available), so force it off
#             set(HOPS_ENABLE_TEST OFF CACHE BOOL "Build developer tests." FORCE)
#     endif()
# endif(HOPS_ENABLE_TEST)

# if(HOPS_ENABLE_TEST)
#     if(NOT HOPS_USE_CACHED_TEST_DATA)
#             #no local cached data available, so force it off
#             set(HOPS_ENABLE_TEST OFF CACHE BOOL "Build developer tests." FORCE)
#     endif()
# endif(HOPS_ENABLE_TEST)

if(HOPS_ENABLE_TEST)
    enable_testing()
    add_subdirectory(${TEST_DATA_SOURCE})
endif(HOPS_ENABLE_TEST)


################################################################################
#python package setup
option(HOPS_PYPI_MANAGE_DEPS "Allow pip to download and install python dependencies via PyPi." OFF)

if(${CMAKE_VERSION} VERSION_GREATER "3.12.0")
    find_package (Python3 COMPONENTS Interpreter Development)
    if (Python3_EXECUTABLE)
        set(PY_EXE ${Python3_EXECUTABLE})
    endif(Python3_EXECUTABLE)
else()
  #if CMAKE version < 3.12 use old style find package to figure out where python is
  find_package(PythonInterp REQUIRED)
  find_package(PythonLibs REQUIRED)

  if (PYTHON_EXECUTABLE)
      set(PY_EXE ${PYTHON_EXECUTABLE})
  endif (PYTHON_EXECUTABLE)
endif()


if(PY_EXE)

    #figure out the python version and appropriate install path, so we can add it to PYTHONPATH
    #also figure out if we should use distutils or setuptools/pip
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pyinfo.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
    execute_process(COMMAND ${PY_EXE} "${CMAKE_CURRENT_BINARY_DIR}/pyinfo.py"  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE PYTHON_CONFIG)
    string(STRIP "${PYTHON_CONFIG}" PYTHON_CONFIG)
    message(STATUS "Python configuration (vers,prefix,setup,need_pip,have_pip) = ${PYTHON_CONFIG}")
    set(PYTHON_CONFIG_SETTINGS ${PYTHON_CONFIG} )

    list(GET PYTHON_CONFIG_SETTINGS 0 PYTHON_VERS)
    list(GET PYTHON_CONFIG_SETTINGS 1 PYTHON_SITE_PREFIX)
    list(GET PYTHON_CONFIG_SETTINGS 2 PYTHON_SETUP)
    list(GET PYTHON_CONFIG_SETTINGS 3 PYTHON_NEED_PIP)
    list(GET PYTHON_CONFIG_SETTINGS 4 PYTHON_HAVE_PIP)
    set( PYTHON_MODULE_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_PREFIX}")

else()

    #no python found so turn off pybind11 too
    set(HOPS_USE_PYBIND11 OFF CACHE BOOL "Build python bindings with pybind11" FORCE)

endif(PY_EXE)


################################################################################
#call build in the subdirectories
add_subdirectory(${SHSOURCE}) #shell scripts
add_subdirectory(${EXTERN_HLIBS}) #external (mostly header-only) dependencies, but date (tz) library must be built
add_subdirectory(${CSOURCE}) #mostly hops3 source
add_subdirectory(${CPPSOURCE}) #mostly hops4 source

if(PY_EXE)
    # add the python source dir
    if("${PYTHON_NEED_PIP}" STREQUAL "false")
        add_subdirectory(${PYSOURCE})
    else()
        if("${PYTHON_HAVE_PIP}" STREQUAL "true")
        add_subdirectory(${PYSOURCE})
        else()
        message(WARNING "Python ${PYTHON_VERS} requires pip for proper packaging of local hops tools. Install pip for these to be built.")
        endif()
    endif()
endif()

################################################################################
#find and configure perl if available
find_package(Perl)
if(PERL_FOUND)
    add_subdirectory(${PERLSOURCE})
endif(PERL_FOUND)


#build docs
add_subdirectory(doc)
