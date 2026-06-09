#include "MHO_PythonBackendSelector.hh"

#include "MHO_Message.hh"

#if defined(USE_EMBEDDED_PYTHON)
    #include "MHO_PyControlEvaluator.hh"
    #include "MHO_PythonPluginInterface.hh"
#elif defined(USE_PYTHON_SUBPROCESS)
    #include "MHO_SubprocessPyControlEvaluator.hh"
#endif

namespace hops
{

MHO_PythonControlEvaluatorFn MakePythonControlEvaluator()
{
#if defined(USE_EMBEDDED_PYTHON)
    //the in-process control evaluator requires a running interpreter; start it
    //now if it has not been initialized yet
    MHO_PythonPluginInterface::EnsureInitialized();
    return &MHO_PyControlEvaluator::Evaluate;
#elif defined(USE_PYTHON_SUBPROCESS)
    return &MHO_SubprocessPyControlEvaluator::Evaluate;
#else
    //no python backend compiled in
    return MHO_PythonControlEvaluatorFn();
#endif
}

} // namespace hops
