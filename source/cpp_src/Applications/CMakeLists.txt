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
)

set(SOURCE_BASENAMES
    hops2keys
    mark42hops
    hops2json
    CompareCorFiles
    ComparePcal
    fplot4
    alist4
    fourfit4
)

set(TEST_BASENAMES
    TestMultiObjectStreaming
    TestSignalFFT
    TestPCalExtraction
)

if(HOPS_USE_PYBIND11)
    hops_internal_include_directories(../Plugins/PythonBindings/Containers/include)
    hops_internal_include_directories(../Plugins/PythonBindings/Operators/include)
    list(APPEND PYBIND_APP_LIBS pyMHO_Containers pyMHO_Operators pybind11::embed)
    add_cflag(USE_PYBIND11)
    list(APPEND HOPS_APP_LIBS ${PYBIND_APP_LIBS})
endif(HOPS_USE_PYBIND11)


foreach( BASENAME ${SOURCE_BASENAMES} )
    add_executable( ${BASENAME} ${BASENAME}.cc )
    target_link_libraries( ${BASENAME} ${HOPS_APP_LIBS})
endforeach( BASENAME )

hops_install_executables( ${SOURCE_BASENAMES})

# #do fourfit4 separately since it needs a build time stamp
# add_executable(fourfit4 fourfit4.cc )
# target_compile_options(fourfit4 PRIVATE -DHOPS_BUILD_TIME=${BUILD_TIME})
# target_link_libraries(fourfit4 ${HOPS_APP_LIBS})
# hops_install_test_executables( fourfit4 )

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
