#declare header paths ##########################################################
hops_internal_include_directories()


#list source files ############################################################
set(PYCALIBRATION_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PyCalibrationModuleDefinition.cc
)

#set libraries to link against ################################################
set(PYCALIBRATION_LIBS
    MHO_Message
    MHO_Utilities
    MHO_Containers
    MHO_Operators
    MHO_Calibration
    pyMHO_Containers
    pyMHO_Operators
)

#create and install the python module #########################################
pybind11_add_module(pyMHO_Calibration SHARED ${PYCALIBRATION_SOURCEFILES})

# Required to suppress pybind11 visibility warnings (see pybind11 FAQ)
target_compile_options(pyMHO_Calibration PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Calibration PRIVATE ${PYCALIBRATION_LIBS})

install(TARGETS pyMHO_Calibration DESTINATION ${PYTHON_MODULE_INSTALL_DIR})

if(HOPS_ENABLE_TEST)
    # add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
