#cmake_minimum_required(VERSION 3.0)
cmake_minimum_required(VERSION 2.8)

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
include(FindGFORTRAN)
include(FindPGPLOT)
include(UseLATEX)

if(${CMAKE_VERSION} VERSION_GREATER "3.11.0")
cmake_policy(SET CMP0074 OLD)
endif()


message(STATUS "Architecture is ${CMAKE_SYSTEM_PROCESSOR}")
set( DEFAULT_INSTALL_DIR  "${CMAKE_CURRENT_BINARY_DIR}/../${CMAKE_SYSTEM_PROCESSOR}-${MODULE_VERSION_NUMBER}" )

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${DEFAULT_INSTALL_DIR}" CACHE PATH "default install path" FORCE )
endif()

#define install directories
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/config")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/data")
set(DOC_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/doc")
set(LOG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/log")
set(SHARE_TEXT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/text")

file(MAKE_DIRECTORY ${INCLUDE_INSTALL_DIR})
file(MAKE_DIRECTORY ${LIB_INSTALL_DIR})
file(MAKE_DIRECTORY ${BIN_INSTALL_DIR})
file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
file(MAKE_DIRECTORY ${DATA_INSTALL_DIR})
file(MAKE_DIRECTORY ${DOC_INSTALL_DIR})
file(MAKE_DIRECTORY ${LOG_INSTALL_DIR})
file(MAKE_DIRECTORY ${SHARE_TEXT_INSTALL_DIR})

hops_module_paths(Hops)

#compiler flags
#hops_require_cpp11()
#hops_require_cpp17()
#
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )
#
# set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive" )
# set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive" )

message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION}")

#options for triggering additional dependencies
#option( HOPS_USE_OPENCL "Build OpenCL support." OFF)
option( HOPS_ENABLE_TEST "Build developer tests." ON)
option( HOPS_USE_ROOT "Build ROOT based utilities." OFF)



#convenience variables
set(CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
set(CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
set(SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
set(PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)

################################################################################
add_subdirectory(${CSOURCE})
add_subdirectory(${CPPSOURCE})
add_subdirectory(doc)

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
