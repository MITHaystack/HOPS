#list header files #############################################################
set (PYVISITOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_DefaultPythonPlotVisitor.hh
)

set( PYVISITOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_DefaultPythonPlotVisitor.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#set libraries to link agains library (headers only) ###########################
set(PYVISITOR_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators pybind11::embed)

#compile and install library ###################################################
add_library (MHO_pyVisitors SHARED ${PYVISITOR_SOURCEFILES})
target_link_libraries (MHO_pyVisitors ${PYVISITOR_LIBS})

hops_install_headers (${PYVISITOR_HEADERFILES})
hops_install_libraries (MHO_pyVisitors)
