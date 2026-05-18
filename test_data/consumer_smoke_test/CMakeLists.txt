# Standalone consumer project. This is NOT built as part of the HOPS build;
# it's configured/built by test_data/test_hops_consumer.sh against an already-
# installed HOPS prefix, to smoke-test that hops4.pc and HopsConfig.cmake are
# usable by external code.
#
# Builds two binaries against the same source file:
#   probe_cmake      -- linked via find_package(Hops CONFIG)
#   probe_pkgconfig  -- linked via pkg_check_modules(HOPS4 hops4)
#
# Either may be skipped if the corresponding config file isn't installed.

cmake_minimum_required(VERSION 3.10)
project(hops_consumer_smoke CXX)

set(_src "${CMAKE_CURRENT_SOURCE_DIR}/test_consumer.cc")

# ---- find_package(Hops) path ----
find_package(Hops CONFIG QUIET)
if(Hops_FOUND)
    add_executable(probe_cmake ${_src})
    target_link_libraries(probe_cmake PRIVATE
        Hops::MHO_Utilities Hops::MHO_Containers Hops::MHO_Message)
    message(STATUS "consumer_smoke_test: find_package(Hops) -> ${Hops_VERSION}")
else()
    message(STATUS "consumer_smoke_test: find_package(Hops) -- NOT FOUND, skipping probe_cmake")
endif()

# ---- pkg-config path ----
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(HOPS4 QUIET IMPORTED_TARGET hops4)
    if(HOPS4_FOUND)
        add_executable(probe_pkgconfig ${_src})
        target_link_libraries(probe_pkgconfig PRIVATE PkgConfig::HOPS4)
        message(STATUS "consumer_smoke_test: pkg-config hops4 -> ${HOPS4_VERSION}")
    else()
        message(STATUS "consumer_smoke_test: pkg-config hops4 -- NOT FOUND, skipping probe_pkgconfig")
    endif()
else()
    message(STATUS "consumer_smoke_test: pkg-config not available, skipping probe_pkgconfig")
endif()

if(NOT TARGET probe_cmake AND NOT TARGET probe_pkgconfig)
    message(FATAL_ERROR
        "Neither HopsConfig.cmake nor hops4.pc could be located. "
        "Check CMAKE_PREFIX_PATH / PKG_CONFIG_PATH.")
endif()
