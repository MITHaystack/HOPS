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
)

#create and install the python module #########################################
pybind11_add_module(pyMHO_Calibration MODULE ${PYCALIBRATION_SOURCEFILES})

# Required to suppress pybind11 visibility warnings (see pybind11 FAQ)
target_compile_options(pyMHO_Calibration PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Calibration PRIVATE ${PYCALIBRATION_LIBS})

#build-order only (no link): the other python modules must exist for cross-module
#type resolution when pyMHO_Calibration is imported.
add_dependencies(pyMHO_Calibration pyMHO_Containers pyMHO_Operators)

install(TARGETS pyMHO_Calibration DESTINATION ${PYTHON_MODULE_INSTALL_RELDIR})

if(HOPS_ENABLE_TEST)
    # add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
