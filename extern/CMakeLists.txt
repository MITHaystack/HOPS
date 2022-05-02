#we require the extern 'date' library (included as a submodule)
#set the following options for the 'date' library build
# include( FetchContent )
# FetchContent_Declare( date
#   GIT_REPOSITORY https://github.com/HowardHinnant/date.git
#   GIT_TAG        v3.0.1  # adjust tag/branch/commit as needed
# )
# FetchContent_MakeAvailable(date)

set_option(USE_SYSTEM_TZ_DB OFF)
set_option(MANUAL_TZ_DB OFF)
set_option(USE_TZ_DB_IN_DOT OFF)
set_option(BUILD_SHARED_LIBS ON)
set_option(ENABLE_DATE_TESTING OFF)
set_option(DISABLE_STRING_VIEW OFF)
set_option(BUILD_TZ_LIB ON)
set_option(COMPILE_WITH_C_LOCALE OFF)

add_subdirectory(date)

add_subdirectory(json)

if(HOPS_USE_PYBIND11)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11)
endif(HOPS_USE_PYBIND11)
