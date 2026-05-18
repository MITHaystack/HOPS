#list header files #############################################################
set (PYVISITOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DefaultPythonPlotVisitor.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PythonPluginInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyControlEvaluator.hh
)

set( PYVISITOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DefaultPythonPlotVisitor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PythonPluginInterface.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PyControlEvaluator.cc
    )

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#compile and install library ###################################################
add_library (MHO_pyVisitors SHARED ${PYVISITOR_SOURCEFILES})

# PUBLIC: symbols / types reachable from the installed public headers.
# PRIVATE: implementation-only deps that aren't named in the headers.
# NOTE: pyMHO_Containers / pyMHO_Operators are *Python modules*, loaded by
# the interpreter via py::module::import(...). Nothing in MHO_pyVisitors's
# .cc files references their C symbols, so they are intentionally NOT in
# the link line. (Having them here forces install(EXPORT) to demand they
# also be in the export set, which makes no sense for Python modules
# installed to PYTHON_MODULE_INSTALL_DIR.)
target_link_libraries(MHO_pyVisitors
    PUBLIC
        MHO_Message
        MHO_Containers
        MHO_Operators
        MHO_Control
        MHO_Initialization
        pybind11::pybind11           # headers only -- public headers use py::object etc.
    PRIVATE
        MHO_Utilities
        MHO_Calibration
        pybind11::embed              # libpython -- only .cc files call
                                     # initialize_interpreter/finalize_interpreter
)

hops_install_headers (${PYVISITOR_HEADERFILES})
hops_install_libraries (MHO_pyVisitors)
