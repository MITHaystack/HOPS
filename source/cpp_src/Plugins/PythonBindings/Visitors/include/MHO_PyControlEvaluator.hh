#ifndef MHO_PyControlEvaluator_HH__
#define MHO_PyControlEvaluator_HH__

#include <functional>
#include <string>

#include "MHO_ControlConditionEvaluator.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"

#include "pybind11_json/pybind11_json.hpp"
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace hops
{

/*!
 *@file  MHO_PyControlEvaluator.hh
 *@class MHO_PyControlEvaluator
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Evaluates a Python control file (.py) and returns the resulting
 *       control statements in the same mho_json format produced by
 *       MHO_ControlFileParser + MHO_ControlConditionEvaluator.
 *
 * The Python control file must define a top-level function::
 *
 *     from hops_control import PassInfo, Config
 *
 *     def configure(p: PassInfo, cfg: Config):
 *         ...
 *
 * MHO_PyControlEvaluator constructs a PassInfo object (from the current
 * pass metadata in the parameter store) and a Config object (from the
 * control-format dict), calls configure(pass_info, config), and then
 * converts config.to_json() back into the mho_json control_statements
 * consumed by MHO_ParameterManager and MHO_OperatorBuilderManager.
 *
 * The Python interpreter must already be initialised before calling
 * Evaluate() - use MHO_PythonPluginInterface::EnsureInitialized().
 */

class MHO_PyControlEvaluator
{
    public:
        /**
         * @brief Evaluate a Python control script and populate control_statements.
         *
         * @param paramStore     Pointer to the current pass parameter store.
         * @param control_format The canonical control-format dict (loaded from JSON files).
         * @param control_statements Output: populated with the applicable statements.
         * @return true on success, false if the Python script raised an exception.
         */
        static bool Evaluate(MHO_ParameterStore* paramStore, const mho_json& control_format, mho_json& control_statements);

    private:
        /// Build the pass-info dict from the parameter store.
        static mho_json BuildPassInfoDict(MHO_ParameterStore* paramStore);

        /**
         * @brief Core evaluator shared by Evaluate() and the pyMHO_Fringe module.
         * Given an already-resolved Python callable, builds PassInfo/Config,
         * calls fn(pass_info, config), extracts statements, then applies
         * condition filtering and set-string overrides.
         */
        static bool EvaluateCallable(py::object fn, MHO_ParameterStore* paramStore, const mho_json& control_format,
                                     mho_json& control_statements);

        /// Apply condition filtering and command-line set-string overrides in-place.
        static void ApplyConditionFilterAndSetString(MHO_ParameterStore* paramStore, mho_json& control_statements);
};

} // namespace hops

#endif /*! end of include guard: MHO_PyControlEvaluator_HH__ */
