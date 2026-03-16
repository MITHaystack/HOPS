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
 * The fringe-data interface is passed directly as an argument to the Julia
 * function using jlcxx::box<MHO_JlFringeDataInterface*>(ptr).  This creates a
 * CxxWrap-boxed jl_value_t* with julia_owned=false (GC will not free the C++
 * memory), which is then passed via jl_call1.
 *
 * Usage from Julia:
 *   op = HOPS.JlGenericOperator()
 *   HOPS.set_julia_function(op, function(fd)
 *       ps = get_parameter_store(fd)
 *       println(get_by_path(ps, "/ref_station/mk4id"))
 *       return true
 *   end)
 */

// Thread-local pointer to the interface currently executing
// (set during Execute(), cleared afterwards)
inline thread_local MHO_JlFringeDataInterface* g_current_fringe_data = nullptr;

// Returns the full exception message string via sprint(showerror, e).
// Caller must have already checked jl_exception_occurred() != nullptr.
inline std::string jl_exception_message()
{
    jl_value_t* exc        = jl_exception_occurred();
    jl_value_t* sprint_fn  = jl_get_function(jl_base_module, "sprint");
    jl_value_t* showerr_fn = jl_get_function(jl_base_module, "showerror");
    if(!sprint_fn || !showerr_fn) { return jl_typeof_str(exc); }
    jl_value_t* msg = jl_call2(sprint_fn, showerr_fn, exc);
    if(!msg || jl_exception_occurred()) { return jl_typeof_str(exc); }
    return std::string(jl_string_ptr(msg));
}


class MHO_JlGenericOperator : public MHO_Operator
{
    public:
        MHO_JlGenericOperator()
            : fInitialized(false), fFringeData(nullptr), fFringeDataInterface(nullptr),
              fJuliaFunc(nullptr){};

        virtual ~MHO_JlGenericOperator() { delete fFringeDataInterface; };

        void SetFringeData(MHO_FringeData* fdata) { fFringeData = fdata; }

        //! Store the Julia function to invoke on each Execute() call.
        //! The function must accept one argument (the fringe-data interface) and return a Bool.
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
                // Ensure the path has a .jl extension so Julia's include() can find it.
                std::string path = fModulePath;
                if(path.size() < 3 || path.substr(path.size() - 3) != ".jl")
                    path += ".jl";

                std::string include_cmd = "include(\"" + path + "\")";
                jl_eval_string(include_cmd.c_str());
                if(jl_exception_occurred())
                {
                    msg_error("julia_bindings",
                              "Failed to include Julia file \"" << path << "\": "
                                  << jl_typeof_str(jl_exception_occurred()) << eom);
                    jl_exception_clear();
                    return false;
                }

                fJuliaFunc = jl_get_function(jl_main_module, fFunctionName.c_str());
                if(!fJuliaFunc)
                {
                    msg_error("julia_bindings",
                              "Julia function \"" << fFunctionName << "\" not found in Main "
                                  << "after including \"" << path << "\"" << eom);
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

            // Box the C++ interface pointer as a Julia value.
            // jlcxx::box<T*> uses julia_owned=false so the GC will not free
            // the C++ memory when the Julia wrapper is collected.
            jl_value_t* boxed_fd = jlcxx::box< MHO_JlFringeDataInterface* >(fFringeDataInterface).value;
            jl_value_t* result   = nullptr;
            JL_GC_PUSH2(&boxed_fd, &result);

            g_current_fringe_data = fFringeDataInterface;
            result                = jl_call1(fJuliaFunc, boxed_fd);
            g_current_fringe_data = nullptr;

            JL_GC_POP();

            if(jl_exception_occurred())
            {
                msg_error("julia_bindings",
                          "Julia exception in operator (" << fModulePath << ":" << fFunctionName << "): "
                              << jl_exception_message() << eom);
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
