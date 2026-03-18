hops_internal_include_directories()
hops_internal_include_directories(../../../extern/cli11)


set(HOPS_APP_LIBS
    MHO_Message
    MHO_Utilities
    MHO_Vex2JSON
    MHO_Math
    MHO_Containers
    MHO_Operators
    MHO_Calibration
    MHO_MK4Interface
    MHO_Control
    MHO_Initialization
    MHO_Fringe
    MHO_PluginManagement
)

set(SOURCE_BASENAMES
    hops2keys
    mark42hops
    hops2mark4
    hops2json
    hops2flat
    CompareCorFiles
    ComparePcal
    fplot4
    alist4
    fourfit4
    #fringex4
    #blingfit
)

set(TEST_BASENAMES
    TestMultiObjectStreaming
    TestSignalFFT
    TestPCalExtraction
)

if(HOPS_USE_PYBIND11)
    hops_internal_include_directories(../Plugins/PythonBindings/Containers/include)
    hops_internal_include_directories(../Plugins/PythonBindings/Operators/include)
    hops_internal_include_directories(../Plugins/PythonBindings/Visitors/include)
    list(APPEND PYBIND_APP_LIBS pyMHO_Containers pyMHO_Operators MHO_pyVisitors pybind11::embed)
    add_cflag(USE_PYBIND11)
    list(APPEND HOPS_APP_LIBS ${PYBIND_APP_LIBS})
endif(HOPS_USE_PYBIND11)

#add julia deps if available
if(HOPS_USE_JULIA)
    hops_internal_include_directories(../Plugins/JuliaBindings/Containers/include)
    hops_internal_include_directories(../Plugins/JuliaBindings/Operators/include)
    hops_internal_include_directories(../Plugins/JuliaBindings/Visitors/include)
    list(APPEND JULIA_APP_LIBS jlMHO_Containers jlMHO_Operators MHO_jlVisitors JlCxx::cxxwrap_julia JlCxx::cxxwrap_julia_stl)
    list(APPEND HOPS_APP_LIBS ${JULIA_APP_LIBS})
endif(HOPS_USE_JULIA)

if(HOPS_USE_MATPLOTPP AND Gnuplot_FOUND)
    hops_internal_include_directories(../../../extern/matplotplusplus/source/)
    hops_internal_include_directories(../Plugins/MatplotPlugin/include)
    list(APPEND MPP_LIBS MHO_MatplotPlugin matplot)
    add_cflag(USE_MATPLOTPP)
    list(APPEND HOPS_APP_LIBS ${MPP_LIBS})
endif()

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    if( "${BASENAME}" STREQUAL "fourfit4") 
        target_compile_options( ${BASENAME} PUBLIC -DHOPS_BUILD_TIME=${BUILD_TIME})
    endif()
    target_link_libraries( ${BASENAME} ${HOPS_APP_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES})

foreach( BASENAME ${TEST_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_APP_LIBS})
endforeach( BASENAME )

hops_install_test_executables( ${TEST_BASENAMES} )

#fourfit is fourfit4...so install a symlink as: fourfit -> fourfit4
if(HOPS_IS_HOPS4)
    hops_install_symlink( "${BIN_INSTALL_DIR}/fourfit4" "${BIN_INSTALL_DIR}/fourfit" )
    hops_install_symlink( "${BIN_INSTALL_DIR}/fplot4" "${BIN_INSTALL_DIR}/fplot" )
    hops_install_symlink( "${BIN_INSTALL_DIR}/alist4" "${BIN_INSTALL_DIR}/alist" )
endif()

################################################################################
if(HOPS_ENABLE_TEST)
   add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
