#list header files #############################################################
set(JLOPERATOR_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JlGenericOperator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_JuliaOperatorBuilder.hh
)

set(JLOPERATOR_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_JlOperatorsModuleDefinition.cc
)

#declare header paths ##########################################################
hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#set libraries to link against #################################################
set(JLOPERATOR_LIBS MHO_Message MHO_Utilities MHO_Containers MHO_Operators jlMHO_Containers)

add_library(jlMHO_Operators SHARED ${JLOPERATOR_SOURCEFILES})
set_target_properties(jlMHO_Operators PROPERTIES CXX_STANDARD 17)

target_link_libraries(jlMHO_Operators PRIVATE
    ${JLOPERATOR_LIBS}
    JlCxx::cxxwrap_julia
    JlCxx::cxxwrap_julia_stl
)

install(TARGETS jlMHO_Operators DESTINATION ${JULIA_MODULE_INSTALL_DIR})
