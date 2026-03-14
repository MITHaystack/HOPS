# Configure the Julia test script so it knows where the installed .so lives.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_jl_containers.jl.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_jl_containers.jl
    @ONLY
)

install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/test_jl_containers.jl
    DESTINATION ${JULIA_MODULE_INSTALL_DIR}
)

# Example Julia operator function - installed alongside the test script.
# Use with MHO_JlGenericOperator by setting:
#   module_path   = "<install_dir>/example_jl_operator.jl"
#   function_name = "example_jl_operator"
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/example_jl_operator.jl
    DESTINATION ${JULIA_MODULE_INSTALL_DIR}
)
