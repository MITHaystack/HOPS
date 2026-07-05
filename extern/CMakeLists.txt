add_subdirectory(json)

# Eigen is header-only - expose include directory
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/eigen)

if(HOPS_USE_PYBIND11)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pybind11)
endif(HOPS_USE_PYBIND11)

# ----------------------------------------------------------------------------
# Install vendored header-only dependencies under ${INCLUDE_INSTALL_DIR}/hops_extern
# so downstream consumers using hops4.pc / HopsConfig.cmake get matching headers.
# ----------------------------------------------------------------------------
set(HOPS_EXTERN_INCLUDE_INSTALL_DIR "${INCLUDE_INSTALL_DIR}/hops_extern")

# Eigen: install the "Eigen" header tree (and "unsupported" for completeness)
# into hops_extern/eigen/, matching the standard Eigen layout.
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/eigen/Eigen
    DESTINATION ${HOPS_EXTERN_INCLUDE_INSTALL_DIR}/eigen
)
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/eigen/unsupported
    DESTINATION ${HOPS_EXTERN_INCLUDE_INSTALL_DIR}/eigen
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# nlohmann/json + pybind11_json (single_include layout)
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/json/include/
    DESTINATION ${HOPS_EXTERN_INCLUDE_INSTALL_DIR}/json
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# CLI11: a single header.
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/cli11/CLI11.hpp
    DESTINATION ${HOPS_EXTERN_INCLUDE_INSTALL_DIR}/cli11
)

# pybind11 headers (only useful for downstream consumers writing their own
# bindings; harmless otherwise).
if(HOPS_USE_PYBIND11)
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include/pybind11
        DESTINATION ${HOPS_EXTERN_INCLUDE_INSTALL_DIR}/pybind11
        FILES_MATCHING PATTERN "*.h"
    )
endif()

set_option(USE_SYSTEM_TZ_DB ON)
set_option(MANUAL_TZ_DB OFF)
set_option(USE_TZ_DB_IN_DOT OFF)
set_option(BUILD_SHARED_LIBS ON)
set_option(ENABLE_DATE_TESTING OFF)
set_option(DISABLE_STRING_VIEW OFF)
set_option(BUILD_TZ_LIB ON)

#for GCC versions < 5, work around for bug in c++11 impl
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    set_option(COMPILE_WITH_C_LOCALE ON)
else()
    set_option(COMPILE_WITH_C_LOCALE OFF)
endif()

add_subdirectory(date)

# Static twin of the vendored date-tz library (built alongside the shared
# libdate-tz.so when HOPS_BUILD_STATIC_LIBS is ON) so the low-level HOPS libraries
# can be linked into self-contained applications. The vendored CMakeLists defines
# date-tz exactly once (type follows BUILD_SHARED_LIBS, which we force ON), so we
# add a second STATIC target from the same single source. Its compile definitions
# are inherited from the real target via a generator expression so there are no
# duplicated flag literals to drift out of sync. See Review/static-libs-plan.md.
if(HOPS_BUILD_STATIC_LIBS)
    add_library(date-tz_static STATIC ${CMAKE_CURRENT_SOURCE_DIR}/date/src/tz.cpp)
    set_target_properties(date-tz_static PROPERTIES
        OUTPUT_NAME date-tz
        POSITION_INDEPENDENT_CODE ON)
    #tz.cpp only needs the date/include tree; set it explicitly (the vendored
    #target wraps its include dirs in $<BUILD_INTERFACE:...>, which is awkward to
    #copy via genex). The compile definitions (USE_OS_TZDB, AUTO_DOWNLOAD, ...) are
    #plain strings and ARE inherited from the real target so they can't drift.
    target_include_directories(date-tz_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/date/include>
        $<INSTALL_INTERFACE:include>)
    target_compile_definitions(date-tz_static PRIVATE
        $<TARGET_PROPERTY:date-tz,COMPILE_DEFINITIONS>)
    find_package(Threads REQUIRED)
    target_link_libraries(date-tz_static PUBLIC Threads::Threads)
    install(TARGETS date-tz_static
        EXPORT hopsStaticTargets DESTINATION ${LIB_INSTALL_RELDIR})
    set_property(GLOBAL APPEND PROPERTY MODULE_STATIC_TARGETS date-tz_static)
endif()



#Note that gnuplot is a RUNTIME dependency of matplot++
if(HOPS_USE_MATPLOTPP)

    set_option(FETCHCONTENT_QUIET OFF)
    set_option(FETCHCONTENT_FULLY_DISCONNECTED OFF)
    set_option(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

    add_subdirectory(matplotplusplus)
    #install the matplot.so library into hopsTargets so it's part of the
    #same exported set as MHO_MatplotPlugin (which links to it PUBLICly).
    # relative DESTINATION (see hops_install_libraries() in HopsDefaults.cmake)
    # so the exported target stays relocatable/reproducible.
    install(
        TARGETS matplot
        EXPORT hopsTargets
        LIBRARY DESTINATION ${LIB_INSTALL_RELDIR}
    )

    #might as well install the headers too
    set(MATPLOT_INCLUDE_INSTALL_DIR "${INCLUDE_INSTALL_DIR}/matplot")
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/matplotplusplus/source/matplot
        DESTINATION ${MATPLOT_INCLUDE_INSTALL_DIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()
