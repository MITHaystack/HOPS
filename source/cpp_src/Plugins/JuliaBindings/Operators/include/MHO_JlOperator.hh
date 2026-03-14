#ifndef MHO_JlOperator_HH__
#define MHO_JlOperator_HH__

#include "MHO_Operator.hh"

// Julia C API (available via jlcxx)
#include "jlcxx/jlcxx.hpp"
#include "julia.h"

namespace hops
{

/*!
 *@file  MHO_JlOperator.hh
 *@class  MHO_JlOperator
 *@author  J. Barrett - barrettj@mit.edu
 *@brief  Concrete MHO_Operator whose Initialize/Execute are implemented by
 *        Julia callbacks.
 *
 * Unlike pybind11's trampoline pattern (which lets Julia/Python subclass
 * MHO_Operator), CxxWrap does not support Julia-side virtual dispatch.
 * Instead, the Julia user supplies function objects for Initialize and Execute:
 *
 *   op = HOPS.JlOperator()
 *   HOPS.set_initialize_fn(op, () -> true)
 *   HOPS.set_execute_fn(op, () -> begin ... ; true end)
 */

class MHO_JlOperator : public MHO_Operator
{
    public:
        MHO_JlOperator() : fInitFunc(nullptr), fExecFunc(nullptr){};
        virtual ~MHO_JlOperator(){};

        //! Set the Julia function to call for Initialize() (no-arg, returns Bool).
        void SetInitializeFunction(jl_function_t* f) { fInitFunc = f; }

        //! Set the Julia function to call for Execute() (no-arg, returns Bool).
        void SetExecuteFunction(jl_function_t* f) { fExecFunc = f; }

        virtual bool Initialize() override
        {
            if(!fInitFunc) { return true; } // default: success
            jl_value_t* result = jl_call0(fInitFunc);
            if(jl_exception_occurred())
            {
                msg_error("julia_bindings",
                          "Julia exception in Initialize: "
                              << jl_typeof_str(jl_exception_occurred()) << eom);
                jl_exception_clear();
                return false;
            }
            return (result != nullptr) && jl_unbox_bool(result);
        }

        virtual bool Execute() override
        {
            if(!fExecFunc) { return false; }
            jl_value_t* result = jl_call0(fExecFunc);
            if(jl_exception_occurred())
            {
                msg_error("julia_bindings",
                          "Julia exception in Execute: "
                              << jl_typeof_str(jl_exception_occurred()) << eom);
                jl_exception_clear();
                return false;
            }
            return (result != nullptr) && jl_unbox_bool(result);
        }

    private:
        jl_function_t* fInitFunc;
        jl_function_t* fExecFunc;
};

} // namespace hops

#endif /*! end of include guard: MHO_JlOperator */
