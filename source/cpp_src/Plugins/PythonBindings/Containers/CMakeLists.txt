#list header files #############################################################
set (PYCONTAINER_HEADERFILES
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyAxisHelpers.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyTableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyParameterStoreInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PyContainerStoreInterface.hh
)

set( PYCONTAINER_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PyContainersModuleDefinition.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#set libraries to link agains library (headers only) ###########################
set(PYCONTAINER_LIBS MHO_Message MHO_Utilities MHO_Containers)

#create and install the python module
pybind11_add_module(pyMHO_Containers SHARED ${PYCONTAINER_SOURCEFILES})

#we need to add the -fvisibility hidden flag, to avoid warnings of the type
# X declared with greater visibility than the type of its field Y
# see pybind11 FAQ:
# https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
target_compile_options(pyMHO_Containers PRIVATE -fvisibility=hidden)

target_link_libraries(pyMHO_Containers PRIVATE ${PYCONTAINER_LIBS})

install(TARGETS pyMHO_Containers DESTINATION ${PYTHON_MODULE_INSTALL_DIR})

if (HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif (HOPS_ENABLE_TEST)
