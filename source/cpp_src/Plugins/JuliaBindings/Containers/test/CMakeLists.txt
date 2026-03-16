# Julia test/example scripts - installed as plain files.
# Library paths are resolved at runtime via the HOPS_JULIA_MODULES_DIR
# environment variable exported by hopsenv.sh (source it before running).

install(
    PROGRAMS
        ${CMAKE_CURRENT_SOURCE_DIR}/test_jl_containers.jl
        ${CMAKE_CURRENT_SOURCE_DIR}/plot_visibilities.jl
    DESTINATION ${JULIA_MODULE_INSTALL_DIR}
)
