add_subdirectory(json)

if(HOPS_USE_PYBIND11)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/pybind11)
endif(HOPS_USE_PYBIND11)

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

if(HOPS_USE_MATPLOTPP)

#TODO...we should check if the user has matplotplusplus installed locally first! 

add_subdirectory(matplotplusplus)
#install the matplot.so library
install(
    TARGETS matplot
    EXPORT matplotTargets
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}    # for .so
)

#might as well install the headers too
set(MATPLOT_INCLUDE_INSTALL_DIR "${INCLUDE_INSTALL_DIR}/matplot")
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/matplotplusplus/source/matplot
    DESTINATION ${MATPLOT_INCLUDE_INSTALL_DIR}
    FILES_MATCHING PATTERN "*.h"
)

endif(HOPS_USE_MATPLOTPP)
