cmake_minimum_required(VERSION 3.0)
project(Hops)

# Module version
set(MODULE_VERSION_MAJOR 4)
set(MODULE_VERSION_MINOR 0)
set(MODULE_VERSION_PATCH 0)
set(MODULE_VERSION "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH}")
set(MODULE_VERSION_NUMBER "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}${MODULE_VERSION_PATCH}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )
include(HopsDefaults)
include(CMakeDependentOption)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

#define install directories
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/config")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(LOG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/log")
set(SHARE_TEXT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/text")


file(MAKE_DIRECTORY ${INCLUDE_INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_INSTALL_DIR})
file(MAKE_DIRECTORY ${BIN_INSTALL_DIR})
file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
file(MAKE_DIRECTORY ${DATA_INSTALL_DIR})
file(MAKE_DIRECTORY ${LOG_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_TEXT_INSTALL_DIR})

hops_module_paths(Hops)

#compiler flags
hops_require_cpp11()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread" )
message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION}")

#options for triggering additional dependencies
#option( HOPS_USE_OPENCL "Build OpenCL support." OFF)
#option( HOPS_USE_ROOT "Build ROOT based utilities." OFF)
option( HOPS_ENABLE_TEST "Build developer tests." ON)
option( HOSE_USE_ROOT "Build ROOT based utilities." OFF)


#convenience variables
set(CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
set(CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
set(SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
set(PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)




################################################################################
#CONFIGURE AND BUILD OLD C-LIBRARIES FROM HOPS

#config header generation to mimic behavior of autoheader generated hops_config.h
#some of these are probably not necessary
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(STATUS "Machine is BIG_ENDIAN")
    set(WORDS_BIGENDIAN TRUE)
else()
    message(STATUS "Machine is LITTLE_ENDIAN")
    set(WORDS_BIGENDIAN FALSE)
endif()

#mimic a bunch of checks automake does (which AFAIK are not used anywhere)
include(CheckIncludeFile)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)

check_include_file(alloca.h HAVE_ALLOCA_H)
check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(float.h HAVE_FLOAT_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(limits.h HAVE_LIMITS_H)

check_include_file(locale.h HAVE_LOCALE_H)
check_include_file(netdb.h HAVE_NETDB_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(stddef.h HAVE_STDDEF_H)
check_include_file(stdint.h HAVE_STDINT_H)

check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(unistd.h HAVE_UNISTD_H)

set(HOPS_SVN_REV_VALUE 0)
set(HOPS_REVISION_VALUE ${MODULE_VERSION_NUMBER})
set(PACKAGE_VERSION ${MODULE_VERSION_NUMBER})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/hops_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/hops_config.h)
hops_install_headers(${CMAKE_CURRENT_BINARY_DIR}/hops_config.h)
hops_include_directories(${CMAKE_CURRENT_BINARY_DIR})
hops_add_cflag(HAVE_CONFIG_H)

#mk4 and vex c-libraries from old HOPS
add_subdirectory(${CSOURCE}/mk4stub)
add_subdirectory(${CSOURCE}/mk4util)
add_subdirectory(${CSOURCE}/vex)
add_subdirectory(${CSOURCE}/dfio)
add_subdirectory(${CSOURCE}/fourfit_libs/ffcontrol)
add_subdirectory(${CSOURCE}/fourfit_libs/ffcore)

################################################################################

#source sub directories (in order of dependence...this is important!)
add_subdirectory(${CPPSOURCE}/Array)
add_subdirectory(${CPPSOURCE}/Core)
add_subdirectory(${CPPSOURCE}/MK4Interface)
# add_subdirectory(${CPPSOURCE}/Bindings)
add_subdirectory(${CPPSOURCE}/Test)


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

  #now add the python source dir
  add_subdirectory(${PYSOURCE})

endif (PYTHON_EXECUTABLE)

#configure environment set-up script
configure_file( ${SHSOURCE}/hopsenv.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/hopsenv.sh @ONLY)
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hopsenv.sh DESTINATION ${BIN_INSTALL_DIR} )
