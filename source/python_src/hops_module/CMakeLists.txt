#python package setup ##########################################################
# These packages install as plain *.py file copies (no build-time interpreter
# required). PYTHON_MODULE_INSTALL_RELDIR is always defined by the top-level
# CMakeLists (queried from the interpreter when present, else a version-independent
# fallback), so these stage correctly even in a python-free build.

# Pure-python packages: safe to ship regardless of pybind/embedded support. The
# libpython-free subprocess backend drives hops_control / hops_visualization via
# the user's runtime python3.
foreach(pkg hops_control hops_visualization hops_io)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${pkg}"
            DESTINATION "${PYTHON_MODULE_INSTALL_RELDIR}"
            FILES_MATCHING
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
endforeach()

# The top-level 'hops' package imports the compiled pyMHO_* bindings, so only ship
# it when those bindings are actually built (otherwise 'import hops' would fail).
if(HOPS_USE_PYBIND11)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hops"
            DESTINATION "${PYTHON_MODULE_INSTALL_RELDIR}"
            FILES_MATCHING
            PATTERN "*.py"
            PATTERN "__pycache__" EXCLUDE)
endif(HOPS_USE_PYBIND11)
