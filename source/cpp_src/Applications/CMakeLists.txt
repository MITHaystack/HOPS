hops_internal_include_directories()
hops_internal_include_directories(../../../extern/cli11)


set(HOPS_TEST_LIBS
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
)

set(SOURCE_BASENAMES
    TestMultiObjectStreaming
    # TestConvertMk4Visibilities
    hopskeys
    mark42hops
    hops2json
    vex2json
    TestSignalFFT
    CompareCorFiles
    TestPCalExtraction
    fourfit4
    fplot4
)

#for testing purposes only -- ROOT not intended to be a dependency
if (HOPS_USE_ROOT)
    include(FindROOT)
    find_package(ROOT REQUIRED)
    hops_internal_include_directories(../Plugins/Root/include)
    hops_external_include_directories(${ROOT_INCLUDE_DIRS})
    add_cflag(USE_ROOT)
    list(APPEND HOPS_TEST_LIBS MHO_RootPlugin ${ROOT_LIBRARIES})
endif()


if(HOPS_USE_PYBIND11)
    hops_internal_include_directories(../Plugins/PythonBindings/Containers/include)
    hops_internal_include_directories(../Plugins/PythonBindings/Operators/include)
    list(APPEND PYBIND_TEST_LIBS pyMHO_Containers pyMHO_Operators pybind11::embed)
    add_cflag(USE_PYBIND11)
    list(APPEND HOPS_TEST_LIBS ${PYBIND_TEST_LIBS})
endif(HOPS_USE_PYBIND11)

#add the build time as a CXX flag for all these executables (mainly needed by fourfit)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHOPS_BUILD_TIME=${BUILD_TIME} ")

foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_TEST_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES} )

#fourfit is fourfit4...so install a symlink as: fourfit -> fourfit4
if(HOPS_IS_HOPS4)
    hops_install_symlink( "${BIN_INSTALL_DIR}/fourfit4" "${BIN_INSTALL_DIR}/fourfit" )
    hops_install_symlink( "${BIN_INSTALL_DIR}/fplot4" "${BIN_INSTALL_DIR}/fplot" )
endif()

################################################################################
if(HOPS_ENABLE_TEST)
   add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
