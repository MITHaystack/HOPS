#ifndef MHO_JlGenericOperator_HH__
#define MHO_JlGenericOperator_HH__

#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeData.hh"
#include "MHO_Operator.hh"

#include "MHO_JlFringeDataInterface.hh"

// Julia C API
#include "jlcxx/jlcxx.hpp"
#include "julia.h"

namespace hops
{

/*!
 *@file  MHO_JlGenericOperator.hh
 *@class  MHO_JlGenericOperator
 *@author  J. Barrett - barrettj@mit.edu
 *@brief  Calls a Julia function as part of the fringe-fitting operator pipeline.
 *
 * Design (global-state pattern):
 *   - At Execute() time, the current MHO_JlFringeDataInterface* is stored in
 *     a thread-local global (g_current_fringe_data).
 *   - The no-argument Julia function is then called.
 *   - Inside the Julia function, the user retrieves the interface with:
 *       fd = HOPS.get_current_fringe_data()
 *
 * This avoids the need to box the C++ interface object for jl_call1.
 *
 * Alternative approach: use SetJuliaFunction() and set_execute_fn() to
 * store a pre-wrapped fringe data reference at Initialize() time, then
 * call jl_call0(fJuliaFunc) at Execute() with the interface available
 * via the global.
 *
 * Usage from Julia:
 *   op = HOPS.JlGenericOperator()
 *   HOPS.set_julia_function(op, () -> begin
 *       fd = HOPS.get_current_fringe_data()
 *       ps = get_parameter_store(fd)
 *       println(get_by_path(ps, "/ref_station/mk4id"))
 *       return true
 *   end)
 */

// Thread-local pointer to the interface currently executing
// (set during Execute(), cleared afterwards)
inline thread_local MHO_JlFringeDataInterface* g_current_fringe_data = nullptr;


class MHO_JlGenericOperator : public MHO_Operator
{
    public:
        MHO_JlGenericOperator()
            : fInitialized(false), fFringeData(nullptr), fFringeDataInterface(nullptr),
              fJuliaFunc(nullptr){};

        virtual ~MHO_JlGenericOperator() { delete fFringeDataInterface; };

        void SetFringeData(MHO_FringeData* fdata) { fFringeData = fdata; }

        //! Store the Julia function to invoke on each Execute() call.
        //! The function must accept no arguments and return a Bool.
        void SetJuliaFunction(jl_function_t* f) { fJuliaFunc = f; }

        // Path to a Julia source file and the function name to load from it.
        // Initialize() calls include(module_path) then looks up function_name in Main.
        // These are used when no explicit Julia function pointer has been set via
        // SetJuliaFunction() - i.e., the normal path when built by MHO_JuliaOperatorBuilder.
        void SetModulePath(const std::string& path) { fModulePath = path; }
        void SetFunctionName(const std::string& name) { fFunctionName = name; }
        std::string GetModulePath()   const { return fModulePath; }
        std::string GetFunctionName() const { return fFunctionName; }

        virtual bool Initialize() override
        {
            fInitialized = false;

            // If no function pointer yet, try to load from module_path + function_name.
            // module_path is a path to a Julia source file; function_name is exported
            // at top level (or in Main) after include().
            if(!fJuliaFunc && !fModulePath.empty() && !fFunctionName.empty())
            {
                std::string include_cmd = "include(\"" + fModulePath + "\")";
                jl_eval_string(include_cmd.c_str());
                if(jl_exception_occurred())
                {
                    msg_error("julia_bindings",
                              "Failed to include Julia file \"" << fModulePath << "\": "
                                  << jl_typeof_str(jl_exception_occurred()) << eom);
                    jl_exception_clear();
                    return false;
                }

                fJuliaFunc = jl_get_function(jl_main_module, fFunctionName.c_str());
                if(!fJuliaFunc)
                {
                    msg_error("julia_bindings",
                              "Julia function \"" << fFunctionName << "\" not found in Main "
                                  << "after including \"" << fModulePath << "\"" << eom);
                    return false;
                }
            }

            if(!fJuliaFunc) { return false; }
            if(fFringeData && !fFringeDataInterface)
            {
                fFringeDataInterface = new MHO_JlFringeDataInterface(fFringeData);
            }
            fInitialized = (fFringeDataInterface != nullptr);
            return fInitialized;
        }

        virtual bool Execute() override
        {
            if(!fInitialized || !fJuliaFunc || !fFringeDataInterface) { return false; }

            // Expose the current interface via the thread-local global so that
            // the Julia function can retrieve it with get_current_fringe_data().
            g_current_fringe_data = fFringeDataInterface;

            jl_value_t* result = jl_call0(fJuliaFunc);
            g_current_fringe_data = nullptr;

            if(jl_exception_occurred())
            {
                msg_error("julia_bindings",
                          "Julia exception in operator (" << fModulePath << ":" << fFunctionName << "): "
                              << jl_typeof_str(jl_exception_occurred()) << eom);
                jl_exception_clear();
                return false;
            }
            return (result != nullptr) && jl_unbox_bool(result);
        }

    private:
        bool fInitialized;
        std::string fModulePath;
        std::string fFunctionName;
        MHO_FringeData* fFringeData;
        MHO_JlFringeDataInterface* fFringeDataInterface;
        jl_function_t* fJuliaFunc;
};

} // namespace hops

#endif /*! end of include guard: MHO_JlGenericOperator */
