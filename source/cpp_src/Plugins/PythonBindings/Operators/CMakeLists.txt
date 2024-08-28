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

#set libraries to link agains library (headers only) ###########################
set(PYOPERATOR_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators pyMHO_Containers)

#create and install the python module
pybind11_add_module(pyMHO_Operators SHARED ${PYOPERATOR_SOURCEFILES})

#we need to add the -fvisibility hidden flag, to avoid warnings of the type
# X declared with greater visibility than the type of its field Y
# see pybind11 FAQ:
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyMHO_Operators PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Operators PRIVATE ${PYOPERATOR_LIBS})

install(TARGETS pyMHO_Operators DESTINATION ${PYTHON_MODULE_INSTALL_DIR})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
