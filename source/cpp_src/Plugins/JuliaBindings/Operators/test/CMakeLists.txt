# Example Julia operator function - used with MHO_JlGenericOperator by setting:
#   module_path   = "<install_dir>/example_operator.jl"
#   function_name = "example_operator"
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/example_operator.jl
    DESTINATION ${JULIA_MODULE_INSTALL_DIR}
)
