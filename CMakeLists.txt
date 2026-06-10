################################################################################
#CMAKE version and policies
cmake_minimum_required(VERSION 3.15)

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
set(MODULE_VERSION_PATCH 3)
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

# Enable generation of compile_commands.json (for clangd LSP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
#so make sure it exists and is added to the library paths
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
set(SHARE_VHELP_AEDIT_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/vhelp/aedit")

#Relative companions to the absolute *_INSTALL_DIR vars above (resolved against
#CMAKE_INSTALL_PREFIX at install time). We have to use these (not abs paths) as the
#DESTINATION for every install() rule, for two reasons:
#  1. install(TARGETS ... EXPORT hopsTargets) / install(EXPORT ...) need a
#     relative DESTINATION so the generated HopsTargets*.cmake derive the install
#     prefix from their own on-disk location instead of baking in an absolute
#     CMAKE_INSTALL_PREFIX (relocatable/reproducible CMake package config).
#  2. CPack stages a package by re-running the install rules with
#     CMAKE_INSTALL_PREFIX redirected to a temporary staging tree. Relative
#     DESTINATIONs follow that redirect (and land in the package); ABSOLUTE ones
#     are written to the original baked-in path and silently dropped from the
#     package!
#Computed from the absolute vars so the two can never drift out of sync.
file(RELATIVE_PATH INCLUDE_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${INCLUDE_INSTALL_DIR}")
file(RELATIVE_PATH LIB_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${LIB_INSTALL_DIR}")
file(RELATIVE_PATH LIB64_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${LIB64_INSTALL_DIR}")
file(RELATIVE_PATH BIN_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${BIN_INSTALL_DIR}")
file(RELATIVE_PATH TEST_BIN_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${TEST_BIN_INSTALL_DIR}")
file(RELATIVE_PATH CONFIG_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${CONFIG_INSTALL_DIR}")
file(RELATIVE_PATH DATA_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${DATA_INSTALL_DIR}")
file(RELATIVE_PATH DOC_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${DOC_INSTALL_DIR}")
file(RELATIVE_PATH LOG_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${LOG_INSTALL_DIR}")
file(RELATIVE_PATH PLUGINS_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${PLUGINS_INSTALL_DIR}")
file(RELATIVE_PATH SNAPSHOT_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${SNAPSHOT_INSTALL_DIR}")
file(RELATIVE_PATH OPENCL_KERNEL_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${OPENCL_KERNEL_INSTALL_DIR}")
file(RELATIVE_PATH SHARE_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${SHARE_INSTALL_DIR}")
file(RELATIVE_PATH SHARE_TEXT_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${SHARE_TEXT_INSTALL_DIR}")
file(RELATIVE_PATH SHARE_VHELP_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${SHARE_VHELP_INSTALL_DIR}")
file(RELATIVE_PATH SHARE_VHELP_AEDIT_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${SHARE_VHELP_AEDIT_INSTALL_DIR}")

#Create the install-tree directory skeleton at *install* time, not configure
#time, DESTINATION paths are relative to CMAKE_INSTALL_PREFIX.
install(DIRECTORY DESTINATION include)
install(DIRECTORY DESTINATION lib)
install(DIRECTORY DESTINATION lib64)
install(DIRECTORY DESTINATION bin)
install(DIRECTORY DESTINATION bin/test)
install(DIRECTORY DESTINATION config)
install(DIRECTORY DESTINATION data)
install(DIRECTORY DESTINATION doc)
install(DIRECTORY DESTINATION log)
install(DIRECTORY DESTINATION snapshot)
install(DIRECTORY DESTINATION cl_kernel)
install(DIRECTORY DESTINATION share)
install(DIRECTORY DESTINATION share/text)
install(DIRECTORY DESTINATION share/vhelp)
install(DIRECTORY DESTINATION share/vhelp/aedit)
install(DIRECTORY DESTINATION plugin_scripts)

#developer coverage testing requires lcov and genhtml (install with: apt-get install lcov)
#to generate html report with 'make coverage'
#then generate the .svg badges with:
# build$ ../source/bash_src/coverage_badge.sh ./coverage_hops4.info
# build$ ../source/bash_src/coverage_badge.sh ./coverage_hops3.info
#and copy them to the doc directory
include(HopsCoverage)

#convenience variables
set(CSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/c_src)
set(CPPSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/cpp_src)
set(SHSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/bash_src)
set(PYSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/python_src)
set(PERLSOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/perl_src)
set(EXTERN_HLIBS ${CMAKE_CURRENT_SOURCE_DIR}/extern)

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
#create build timestamp (reported by fourfit fringe plots, legacy behavior).
#
#To keep builds reproducible we derive the string(TIMESTAMP) from SOURCE_DATE_EPOCH
#(should be ok so long as CMake >= 3.8) instead of the wall clock. The value is
#resolved in the following order:
#  1. grab SOURCE_DATE_EPOCH set in the environment (explicit override),
#  2. fall back to commit time of HEAD (normal builds from a git checkout),
#  3. fall back to the contents of source-date.last (release tarballs made with git-archive)
#     NOTE: devs must regenerate it before tagging, use:
#     source ./source/bash_src/source-date.sh | tee ./source-date.last
if(NOT DEFINED ENV{SOURCE_DATE_EPOCH})
    #try to derive the epoch from the last commit
    execute_process(
        COMMAND git -C "${CMAKE_CURRENT_SOURCE_DIR}" log -1 --pretty=%ct
        OUTPUT_VARIABLE SOURCE_DATE_EPOCH_VALUE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE SOURCE_DATE_EPOCH_RESULT)
    if(NOT (SOURCE_DATE_EPOCH_RESULT EQUAL 0 AND SOURCE_DATE_EPOCH_VALUE))
        #no git available (e.g. a git-archive tarball), fall back to the recorded value
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source-date.last")
            file(READ "${CMAKE_CURRENT_SOURCE_DIR}/source-date.last" SOURCE_DATE_EPOCH_VALUE)
            string(STRIP "${SOURCE_DATE_EPOCH_VALUE}" SOURCE_DATE_EPOCH_VALUE)
        endif()
    endif()
    if(SOURCE_DATE_EPOCH_VALUE)
        set(ENV{SOURCE_DATE_EPOCH} "${SOURCE_DATE_EPOCH_VALUE}")
    else()
        message(WARNING "Could not determine SOURCE_DATE_EPOCH; build timestamp "
                        "will reflect the current time and the build will not be reproducible.")
    endif()
endif()
string(TIMESTAMP BUILD_TIME "%Y-%m-%dT%H:%M:%S.0Z" UTC)
message(STATUS "The build timestamp is ${BUILD_TIME}")

################################################################################
#options for triggering additional dependencies
option(HOPS_ENABLE_TEST "Build developer tests." ON)
option(HOPS_ENABLE_UNIT_TESTS "Build developer unit tests" OFF)
option(HOPS_USE_FFTW3 "Use FFTW3 for fast fourier transforms (optional, but highly recommended)" ON)
option(HOPS_USE_DIFXIO "Link against difxio library to read DiFX files" OFF)
option(HOPS_USE_PYBIND11 "Build python bindings with pybind11" ON)
option(HOPS_ENABLE_EMBEDDED_PYTHON "Link against an embedded Python interpreter (enables plug-in operators)" ON)
option(HOPS_USE_MATPLOTPP "Build copy of matplot++ (requires C++17; gnuplot needed only at runtime) for fast plotting" ON)
option(HOPS_USE_MPI "Use MPI (Message Passing Interface) for parallel processing" OFF)
option(HOPS_IS_HOPS4 "Install the HOPS4 version of relevant executables (symlinked to expected names, e.g. fourfit, fplot)" OFF)
option(HOPS_USE_OPENCL "Use OpenCL via C++ wrapper API." OFF)
option(HOPS_USE_CUDA "Build CUDA support." OFF)
option(HOPS_USE_HDF5 "Build HDF5 export support." OFF)

option(HOPS3_PYTHON_EXTRAS "Build python (ctypes) interface to hops3 libraries" ON)
option(HOPS3_USE_CXX "Build HOPS3 libraries using the C++ compiler." OFF)
mark_as_advanced(HOPS3_USE_CXX)

if(HOPS3_USE_CXX)
    hops_add_cflag(HOPS3_USE_CXX)
    hops_add_cxxflag(HOPS3_USE_CXX)
    set(HOPS3_PYTHON_EXTRAS OFF CACHE BOOL "(Disabled when HOPS3_USE_CXX=ON)" FORCE )
endif()

# toggles for HOPS3 only third-party libraries. The HOPS4 (C++) code does not use PGPLOT or GSL.
# Note that some library combinations in HOPS3 cannot be distributed as binaries (e.g. PGPLOT+GSL)
option(HOPS3_USE_PGPLOT "Build PGPLOT-linked HOPS3 legacy plotting apps (aedit/search/fourfit3/cohfit)" ON)
option(HOPS3_USE_GSL "Build GSL-linked HOPS3 apps (cohfit)" ON)


##########################################################################################################
#master switch to disable all MIT license incompatible linking
#DEV NOTE: This flag must be switched to ON when building/releasing binary distributions!
option(HOPS_BDIST_LICENSE_COMPAT_ONLY "Restrict the build to redistributable components." OFF)

#license-compatibility mode (must run before FFTW discovery!)
#and before the extern/ subdirectory is added (since itneeds to cover matplot++'s bundled CImg)
if(HOPS_BDIST_LICENSE_COMPAT_ONLY)
    message(STATUS "=====================================================================================")
    message(STATUS "HOPS_BDIST_LICENSE_COMPAT_ONLY=ON: restricts build to MIT-redistributable components.")
    message(STATUS "Forcing OFF: HOPS_USE_FFTW3, HOPS3_USE_GSL (GPL), HOPS3_USE_PGPLOT (non-free) " )
    message(STATUS "=====================================================================================")
    set(HOPS_USE_FFTW3          OFF CACHE BOOL "Use FFTW3 for fast fourier transforms" FORCE)
    set(HOPS3_USE_PGPLOT        OFF CACHE BOOL "Build PGPLOT-linked HOPS3 legacy plotting apps" FORCE)
    set(HOPS3_USE_GSL           OFF CACHE BOOL "Build GSL-linked HOPS3 apps" FORCE)
    #matplot++'s bundled CImg (we don't use it anyways) auto-detects optional backends, so disable these:
    set(CMAKE_DISABLE_FIND_PACKAGE_FFTW ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_TIFF ON)
    set(CMAKE_DISABLE_FIND_PACKAGE_JPEG ON)
    #no CUDA either
    set(HOPS_USE_CUDA OFF CACHE BOOL "Build CUDA support." FORCE)
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

#link against difxio, and build difxinput2json / difx2hops
if(HOPS_USE_DIFXIO)
    find_package(PkgConfig) # we use pkg-config to find difxio
    pkg_search_module(DIFXIO REQUIRED difxio)
endif()

#find MPI devel if needed
if (HOPS_USE_MPI)
    find_package(MPI REQUIRED)
endif(HOPS_USE_MPI)

option(HOPS_USE_OPENMP "Use OpenMP for shared-memory parallelism (e.g. MBD search SBD loop)" ON)
if(HOPS_USE_OPENMP)
    find_package(OpenMP)
    if(NOT OpenMP_CXX_FOUND)
        message(STATUS "OpenMP not found, disabling HOPS_USE_OPENMP")
        set(HOPS_USE_OPENMP OFF CACHE BOOL "Use OpenMP for shared-memory parallelism" FORCE)
    else()
        hops_add_cxxflag(HOPS_USE_OPENMP)
    endif()
endif(HOPS_USE_OPENMP)

if(HOPS_USE_CUDA)
    hops_add_cxxflag(HOPS_USE_CUDA)
endif(HOPS_USE_CUDA)

include(CheckCXXCompilerFlag)
if(HOPS_USE_MATPLOTPP)
    # Check if compiler supports C++17 (requires cmake > 3.8)
    check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
    if(NOT COMPILER_SUPPORTS_CXX17)
        message(WARNING "C++17 is not supported by this compiler, disabling HOPS_USE_MATPLOTPP")
        set(HOPS_USE_MATPLOTPP OFF CACHE BOOL "Build copy of matplot++ (requires C++17)" FORCE)
    else()
        hops_add_cxxflag(USE_MATPLOTPP)
        # gnuplot is only needed at RUNTIME for matplot++
        # it is not required to compile or link, but let user know if it is missing
        find_package(Gnuplot)
        if(NOT Gnuplot_FOUND)
            message(STATUS "gnuplot not found; building the matplot++ plotter anyway "
                           "(gnuplot is required at runtime to actually render plots).")
        endif()
    endif()
endif()

#explicitly set the command to for vpal library fringe fitter via env var
if(HOPS_IS_HOPS4 OR ( NOT HOPS3_USE_PGPLOT OR NOT HOPS_USE_FFTW3)  )
    set(HOPS_VPAL_FOURFIT "fourfit4") #either HOPS3 is unavailable or we chose fourfit4
else()
    set(HOPS_VPAL_FOURFIT "fourfit")
endif()

################################################################################
#stuff for controlling testing/test-data location
set(TEST_DATA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_data)
#location where test data should be dowloaded to (if the user runs testdata_download_all.sh)
set(HOPS_CACHED_TEST_DATADIR "${DATA_INSTALL_DIR}/test_data" CACHE PATH "Location used to download/cache HOPS test data.")
#actual location to unpack the test data and do the tests
set(HOPS_TEST_DATA_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_data)

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


# All HOPS python content (the pure-python packages and, when built, the pybind11
# extension modules) installs into a single, version-INDEPENDENT directory under
# the install prefix. This is the directory added to PYTHONPATH at runtime
# (hops_pypath.sh, the subprocess runner, and the relative sys.path insert baked
# into installed scripts). python never auto-discovers it by version, so the
# name need not encode the CPython minor. Because it carries no minor, ONE install
# tree can hold pybind11 modules for several minors side by side: CPython selects
# the matching one by the SOABI suffix in the .so filename
# (pyMHO_*.cpython-3X-...so), so a multi-minor bundle needs no custom resolver.
# (This also means a build-time interpreter is not required just to know where to
# put things; only the pybind/embedded/pip paths actually need one.)

set(PYTHON_SITE_PREFIX "lib/python3/site-packages")
set(PYTHON_MODULE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${PYTHON_SITE_PREFIX}")
#relative companion (see the *_RELDIR block near the top); use this as the
#install() DESTINATION so the python modules land in cpack-staged packages.
file(RELATIVE_PATH PYTHON_MODULE_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${PYTHON_MODULE_INSTALL_DIR}")
message(STATUS "Python module install path = ${PYTHON_SITE_PREFIX}")

#Add the install-relative python module subdir so runtime code can self-locate
#the shipped packages/bindings (combined with the install prefix resolved at
#runtime) without the user having to source hops_pypath.sh.
add_compile_definitions(HOPS_PYTHON_SITE_SUBDIR="${PYTHON_SITE_PREFIX}")

if(NOT PY_EXE)
    #no interpreter at build time, so turn off pybind11 (the pure-python packages
    #and the libpython-free subprocess backend still build/install without one)
    set(HOPS_USE_PYBIND11 OFF CACHE BOOL "Build python bindings with pybind11" FORCE)
endif()

################################################################################
# The embedded interpreter requires HOPS_USE_PYBIND11=ON, so force it off if PYBIND11 is OFF
if(NOT HOPS_USE_PYBIND11)
    if(HOPS_ENABLE_EMBEDDED_PYTHON)
        message(STATUS "HOPS_USE_PYBIND11=OFF: forcing HOPS_ENABLE_EMBEDDED_PYTHON=OFF")
        set(HOPS_ENABLE_EMBEDDED_PYTHON OFF CACHE BOOL
            "Link an embedded CPython interpreter." FORCE)
    endif()
endif()

# Embedded interpreter needs a shared libpython, fail out if not found
if(HOPS_ENABLE_EMBEDDED_PYTHON AND NOT Python3_Development_FOUND)
    message(FATAL_ERROR
        "HOPS_ENABLE_EMBEDDED_PYTHON=ON but no Python 'Development' component "
        "(shared libpython) was found. Install the python dev package")
endif()

################################################################################
#reproducible-build settings (strip absolute build/source paths from binaries).
#Must run before the subdirectory builds so externs inherit the flags too.
include(HopsReproducible)

################################################################################
#Restrict Eigen to its MPL-2.0 (and more-permissive) code paths only
hops_add_cxxflag(EIGEN_MPL2_ONLY)

################################################################################
#call build in the subdirectories
add_subdirectory(${SHSOURCE}) #shell scripts
add_subdirectory(${CSOURCE}) #mostly hops3 source
add_subdirectory(${EXTERN_HLIBS}) #external (mostly header-only) dependencies, but date (tz) library must be built

################################################################################
#compiler settings (C++11 standard is the default) -- overrides anything set in EXTERN_HLIBS
set(HOPS_CXX_STANDARD "11" CACHE STRING "C++ standard to use (11, 17, or 20)")
set_property(CACHE HOPS_CXX_STANDARD PROPERTY STRINGS "11" "17" "20")
if(HOPS_CXX_STANDARD STREQUAL "11")
    hops_require_cpp11()
elseif(HOPS_CXX_STANDARD STREQUAL "17")
    hops_require_cpp17()
else()
    hops_require_cpp20()
endif()
message(STATUS "Using compiler ${COMPILER_ID} ${COMPILER_VERSION} for HOPS4")
add_subdirectory(${CPPSOURCE}) #mostly hops4 source

# NOTE: The hops python modules install as plain file copies (see
# source/python_src/*/CMakeLists.txt), so no pip/setuptools/etc (and no build-time
# interpreter) is required to stage the pure-python packages. The parts that DO
# need an interpreter (pip dependency management, the pybind plugins, the python
# unit tests) are guarded by PY_EXE inside that subdirectory.
add_subdirectory(${PYSOURCE})

################################################################################
#find and configure perl if available
find_package(Perl)
if(PERL_FOUND)
    add_subdirectory(${PERLSOURCE})
endif(PERL_FOUND)

###############################################################################
# Java ghetto -- we presume these pieces may be moved...  but for the moment we
# collect it all in one place.  ALMA development started with Java 8, so unless
# things have gone incompatible in Java-land, that version or later suffices.
# GBC has Java 25 by default and it still works.  Note that jarfiles are binary
# compatible with any architecture so the lib64 bit isn't needed.
#
# https://cmake.org/cmake/help/latest/module/FindJava.html#module:FindJava
# https://cmake.org/cmake/help/latest/module/UseJava.html#module:UseJava

# NOTE: JAVA 8 doesn't support reproducible builds (file ordering, SOURCE_DATE_EPOCH)
# so builds involving JAVA will not be bit-for-bit reproducible (although they may still portable)
# For now we gate this all behind HOPS_USE_JAVA, and default to "OFF"
# JPB - Do we want to require JAVA 21/22+ for reproducibility?
# My guess is that if you care about using vex2xml, this is probably not important to you.

option(HOPS_USE_JAVA "Allow Java for VEX2XML." OFF)
if(HOPS_USE_JAVA)
    option(HOPS_VEX2XML3 "Provide older jars commons-cli-1.2/antlr-3.5.2 version" ON)
    option(HOPS_VEX2XML4 "Provide newer jars commons-cli-1.4/antlr-4.8 version" OFF)
    find_package(Java 8)
    if(${Java_FOUND})
        include(UseJava)
        message(STATUS "Java ${Java_VERSION} found for VEX2XML")
        # for the moment re-use the existing lib dir
        set(JAVA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
        #relative dir companion (see *_INSTALL_RELDIR block above) so the java
        #install() rules stay relocatable/CPack-friendly (abs dirs will get dropped from the distrib)
        file(RELATIVE_PATH JAVA_INSTALL_RELDIR "${CMAKE_INSTALL_PREFIX}" "${JAVA_INSTALL_DIR}")
        set(JAVASOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/java_src)
        add_subdirectory(${JAVASOURCE})
    endif()
endif()
if(HOPS_VEX2XML3)
    message(STATUS "Making commons-cli-1.2/antlr-3.5.2 version")
endif()
if(HOPS_VEX2XML4)
    message(STATUS "Making commons-cli-1.4/antlr-4.8 version")
endif()
################################################################################

#build docs
add_subdirectory(doc)

################################################################################
# Generate pkg-config (.pc) files and CMake package-config (HopsConfig.cmake)
# so external projects can locate HOPS headers and libraries.
include(HopsPkgConfig)
hops_generate_all_pkgconfig()
hops_generate_cmake_config()

################################################################################
# Binary distribution packaging via CPack...finally!
#
# The TGZ generator stages every install() rule into a clean tree and tars it.
# Because the installed binaries/libs carry $ORIGIN-relative RUNPATHs (see the
# relocatable-build setup above), the resulting tarball is relocatable: extract
# it anywhere and run bin/* directly. However, external shared libraries (fftw3, pgplot,
# X11, gfortran, libpng, zlib, ...) are NOT(!!) bundled and must be present on the
# target system as runtime prerequisites, (probably we should provide a helper install-deps.sh for supported distros)
# That also goes for any optional library deps, like difxio, etc.
#
# DEV: Usage (from the build dir, after configure):
#   cpack -G TGZ
# produces: hops-<version>-<arch>.tar.gz
set(CPACK_PACKAGE_NAME "hops")
set(CPACK_PACKAGE_VENDOR "MIT Haystack Observatory")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HOPS - Haystack Observatory Postprocessing System (VLBI fringe fitting)")
set(CPACK_PACKAGE_VERSION_MAJOR "${MODULE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MODULE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${MODULE_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${HOPS_VERSION_NUMBER}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# <name>-<version>-<arch>.tar.gz, with everything under a matching top dir.
set(CPACK_PACKAGE_FILE_NAME "hops-${HOPS_VERSION_NUMBER}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_GENERATOR "TGZ")

# The package contains whatever the install() rules emit. Note this includes
# the developer test binaries under bin/test when HOPS_ENABLE_TEST is ON;
# We may want to set -DHOPS_ENABLE_TEST=OFF to leave them out...but that's debatable...

# Reproducible tarball: the archive generator ought to follow SOURCE_DATE_EPOCH
# for the mtime stamped on archive members.
set(CPACK_ARCHIVE_THREADS 1)
include(CPack)
