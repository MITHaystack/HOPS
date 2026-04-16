hops_internal_include_directories()

set(PYFRINGE_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PyFringeModuleDefinition.cc
)

set(PYFRINGE_LIBS
    MHO_Fringe          # MHO_FringePass, MHO_ControlConditionEvaluator, MHO_ControlFileParser
    pybind11::module
)

pybind11_add_module(pyMHO_Fringe SHARED ${PYFRINGE_SOURCEFILES})

# Suppress the visibility warnings pybind11 modules commonly trigger
target_compile_options(pyMHO_Fringe PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Fringe PRIVATE ${PYFRINGE_LIBS})

install(TARGETS pyMHO_Fringe DESTINATION ${PYTHON_MODULE_INSTALL_DIR})
