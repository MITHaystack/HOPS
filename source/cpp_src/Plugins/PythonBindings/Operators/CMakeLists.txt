#list header files #############################################################
set (PYOPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyGenericOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PythonOperatorBuilder.hh
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyUnaryTableOperator.hh
)

set( PYOPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PyOperatorsModuleDefinition.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#set libraries to link against (headers only). pyMHO_Containers is NOT linked:
#it is a MODULE (python extension), unlinkable, and exports no C++ symbols we
#use, only the headers. Cross-module pybind type sharing happens at 
#python import time, not via C++ linking.
set(PYOPERATOR_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators)

#create and install the python module
pybind11_add_module(pyMHO_Operators MODULE ${PYOPERATOR_SOURCEFILES})

#we need to add the -fvisibility hidden flag, to avoid warnings of the type
# X declared with greater visibility than the type of its field Y
# see pybind11 FAQ:
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyMHO_Operators PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Operators PRIVATE ${PYOPERATOR_LIBS})

#build-order only (no link): ensure the container module .so exists so that
#`import pyMHO_Operators` can resolve container types at runtime.
add_dependencies(pyMHO_Operators pyMHO_Containers)

install(TARGETS pyMHO_Operators DESTINATION ${PYTHON_MODULE_INSTALL_RELDIR})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
