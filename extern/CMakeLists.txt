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



if(HOPS_USE_MATPLOTPP AND Gnuplot_FOUND)

    set_option(FETCHCONTENT_QUIET OFF)
    set_option(FETCHCONTENT_FULLY_DISCONNECTED OFF)
    set_option(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

    #TODO...we should check if the user has matplotplusplus installed locally first! 
    add_subdirectory(matplotplusplus)
    #install the matplot.so library into hopsTargets so it's part of the
    #same exported set as MHO_MatplotPlugin (which links to it PUBLICly).
    install(
        TARGETS matplot
        EXPORT hopsTargets
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    )

    #might as well install the headers too
    set(MATPLOT_INCLUDE_INSTALL_DIR "${INCLUDE_INSTALL_DIR}/matplot")
    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/matplotplusplus/source/matplot
        DESTINATION ${MATPLOT_INCLUDE_INSTALL_DIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()
