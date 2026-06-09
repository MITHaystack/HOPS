#ifndef MHO_SubprocessPyControlEvaluator_HH__
#define MHO_SubprocessPyControlEvaluator_HH__

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file  MHO_SubprocessPyControlEvaluator.hh
 *@class MHO_SubprocessPyControlEvaluator
 *@author J. Barrett - barrettj@mit.edu
 *@brief No-embed (subprocess) evaluator for Python control files. Exposes the
 *       SAME functor signature MHO_FringePass expects as the embedded
 *       MHO_PyControlEvaluator::Evaluate, but runs the user's control script in
 *       a separate python3 process (no libpython linked) via
 *       `python -m hops_control <request.json>`.
 *
 * The pass-info construction and the condition-filter / set-string
 * post-processing are the SAME pybind-free logic the embedded path uses
 * (MHO_ControlEvaluatorSupport), so the two backends produce identical
 * control_statements for a given control file + pass.
 */

class MHO_SubprocessPyControlEvaluator
{
    public:
        /**
         * @brief Evaluate a Python control script in a subprocess and populate
         * control_statements.
         *
         * @param paramStore         current pass parameter store (provides the
         *                            control-file path and pass metadata).
         * @param control_format     canonical control-format dict.
         * @param control_statements output: applicable statements.
         * @return true on success, false if the subprocess could not be run or
         *         the Python evaluation reported an error.
         */
        static bool Evaluate(MHO_ParameterStore* paramStore, const mho_json& control_format, mho_json& control_statements);
};

} // namespace hops

#endif /*! end of include guard: MHO_SubprocessPyControlEvaluator_HH__ */
