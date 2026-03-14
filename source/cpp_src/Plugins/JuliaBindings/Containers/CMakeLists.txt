#list header files #############################################################
set(JLCONTAINER_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlTableContainer.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlParameterStoreInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlContainerStoreInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlScanStoreInterface.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlFringeDataInterface.hh
)

set(JLCONTAINER_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_JlContainersModuleDefinition.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#set libraries to link against #################################################
set(JLCONTAINER_LIBS MHO_Message MHO_Utilities MHO_Containers)

# Build as a regular shared library (not a pybind11 extension module)
add_library(jlMHO_Containers SHARED ${JLCONTAINER_SOURCEFILES})
set_target_properties(jlMHO_Containers PROPERTIES CXX_STANDARD 17)

target_link_libraries(jlMHO_Containers PRIVATE
    ${JLCONTAINER_LIBS}
    JlCxx::cxxwrap_julia
    JlCxx::cxxwrap_julia_stl
)

install(TARGETS jlMHO_Containers DESTINATION ${JULIA_MODULE_INSTALL_DIR})

add_subdirectory(test)
