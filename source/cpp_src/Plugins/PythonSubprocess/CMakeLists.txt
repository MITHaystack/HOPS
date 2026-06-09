hops_internal_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PYSUBPROC_HEADERFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PythonSubprocessContract.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_PythonSubprocessRunner.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SubprocessPyControlEvaluator.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MHO_SubprocessPythonPlotVisitor.hh
)

set(PYSUBPROC_SOURCEFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_PythonSubprocessRunner.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SubprocessPyControlEvaluator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MHO_SubprocessPythonPlotVisitor.cc
)

#nlohmann json (header-only) for the JSON request/response payloads
include_directories(SYSTEM ${EXTERN_HLIBS}/json/include)

add_library(MHO_PySubprocess SHARED ${PYSUBPROC_SOURCEFILES} ${PYSUBPROC_HEADERFILES})

# This component is deliberately pybind-free / libpython-free: it shells out to
# the user's python3. It needs the directory/parameter utilities, the messaging
# library, the control logic (shared evaluator support), and the containers
# (MHO_FringeData / MHO_FringePlotVisitor for the plot visitor).
target_link_libraries(MHO_PySubprocess MHO_Utilities MHO_Message MHO_Control MHO_Containers)

# Marks the subprocess backends as compiled-in (consumed by fourfit4 / the plot
# factory in the no-embed wiring; see python-refactor.txt task 6).
target_compile_definitions(MHO_PySubprocess PUBLIC USE_PYTHON_SUBPROCESS)

# Bake the install-relative python site-packages subdir so the runtime can build
# the child PYTHONPATH the same way source/bash_src/hops_pypath.sh.in does
# (<install_prefix>/<PYTHON_SITE_PREFIX>), resolved against the install prefix
# at runtime for relocatability.
if(PYTHON_SITE_PREFIX)
    target_compile_definitions(MHO_PySubprocess PRIVATE HOPS_PYTHON_SITE_SUBDIR="${PYTHON_SITE_PREFIX}")
endif()

hops_install_headers(${PYSUBPROC_HEADERFILES})
hops_install_libraries(MHO_PySubprocess)

if(HOPS_ENABLE_TEST)
    add_subdirectory(test)
endif(HOPS_ENABLE_TEST)
