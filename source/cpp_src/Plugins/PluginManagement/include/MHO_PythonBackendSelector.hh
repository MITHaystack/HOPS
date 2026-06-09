#ifndef MHO_PythonBackendSelector_HH__
#define MHO_PythonBackendSelector_HH__

#include <functional>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file  MHO_PythonBackendSelector.hh
 *@author J. Barrett - barrettj@mit.edu
 *@brief Small bridge that lets the (pybind-free) applications obtain a Python
 *       control-file evaluator without referencing the embedded or subprocess
 *       backend classes directly.
 *
 * The applications do NOT receive the USE_EMBEDDED_PYTHON / USE_PYTHON_SUBPROCESS
 * compile definitions and do NOT link the python backends (those live behind
 * MHO_OptionalPlugins, linked PRIVATE by MHO_PluginManagement). This factory
 * function is compiled inside MHO_PluginManagement, so it sees the backend defs
 * and resolves the backend symbols there, returning a plain std::function the
 * app can hand to MHO_FringePass::SetPythonControlEvaluator().
 */

//matches MHO_FringePass::ControlEvaluatorFn
using MHO_PythonControlEvaluatorFn = std::function< bool(MHO_ParameterStore*, const mho_json&, mho_json&) >;

/**
 * @brief Build the Python control-file evaluator for the compiled backend.
 *
 * - Embedded build: ensures the in-process interpreter is initialised and
 *   returns the embedded evaluator.
 * - Subprocess build: returns the subprocess (python3) evaluator.
 * - No python backend: returns an empty std::function (caller should report
 *   that Python control files are unsupported in this build).
 */
MHO_PythonControlEvaluatorFn MakePythonControlEvaluator();

} // namespace hops

#endif /*! end of include guard: MHO_PythonBackendSelector_HH__ */
