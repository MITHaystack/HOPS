hops_internal_include_directories("./")

set(FFIT_LIBS
    MHO_Message
    MHO_Utilities
    vex2json
    MHO_Math
    MHO_Containers
    MHO_Snapshot
    MHO_Operators
    MHO_Calibration
    MHO_Control
    MHO_Initialization
)

set(SOURCE_BASENAMES
    ffit
)

set(ffit_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ffit.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/configure_data_library.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/init_and_exec_operators.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/extract_vex_info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/calculate_fringe_info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/fill_plot_data.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/precalculate_quantities.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/parse_command_line.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/basic_fringe_search.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/construct_plot_data.cc
)

#for testing purposes only -- ROOT not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_internal_include_directories(../../Plugins/Root/include)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND FFIT_LIBS MHO_RootPlugin ${ROOT_LIBRARIES})
endif()


if(HOPS_USE_PYBIND11)
    hops_internal_include_directories(../../Plugins/PythonBindings/Containers/include)
    hops_internal_include_directories(../../Plugins/PythonBindings/Operators/include)
    list(APPEND PYBIND_TEST_LIBS pyMHO_Containers pyMHO_Operators pybind11::embed)
    add_cflag(USE_PYBIND11)
    list(APPEND FFIT_LIBS ${PYBIND_TEST_LIBS})
    # #temp work around until we do this properly
    # install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/ff_plot_test.py" DESTINATION ${PYTHON_MODULE_INSTALL_DIR})
endif(HOPS_USE_PYBIND11)

add_executable( ffit ${ffit_SOURCEFILES} )
target_link_libraries( ffit ${FFIT_LIBS})

hops_install_executables( ${SOURCE_BASENAMES} )

################################################################################

# if(HOPS_ENABLE_TEST)
#    add_subdirectory(test)
# endif(HOPS_ENABLE_TEST)
