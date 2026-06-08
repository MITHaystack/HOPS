#ifndef MHO_ControlEvaluatorSupport_HH__
#define MHO_ControlEvaluatorSupport_HH__

#include "MHO_ControlConditionEvaluator.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file  MHO_ControlEvaluatorSupport.hh
 *@class MHO_ControlEvaluatorSupport
 *@author J. Barrett - barrettj@mit.edu
 *@brief Pure-C++ (pybind-free) helpers shared by the Python control-file
 *       evaluators. Both the embedded (MHO_PyControlEvaluator) and the
 *       subprocess (MHO_SubprocessPyControlEvaluator) backends build the same
 *       pass-info dict from the parameter store and apply the same condition
 *       filtering + command-line set-string overrides to the statements the
 *       Python `configure()` produces. Keeping this logic here (no pybind, no
 *       libpython) lets the subprocess component reuse it without depending on
 *       the pybind11 plugins.
 */

class MHO_ControlEvaluatorSupport
{
    public:
        /**
         * @brief Build the pass-info dict (baseline / station codes / source /
         * fgroup / scan / polprod) handed to the Python configure() callable.
         */
        static mho_json BuildPassInfoDict(MHO_ParameterStore* paramStore);

        /**
         * @brief Filter the raw control statements by the current pass
         * conditions and append any command-line 'set' overrides, in place.
         * Mirrors the behavior of the native MHO_ControlFileParser path.
         */
        static void ApplyConditionFilterAndSetString(MHO_ParameterStore* paramStore, mho_json& control_statements);
};

} // namespace hops

#endif /*! end of include guard: MHO_ControlEvaluatorSupport_HH__ */
