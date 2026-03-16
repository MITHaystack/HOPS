#include "MHO_FringeFitterFactory.hh"
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"
// #include "MHO_IonosphericFringeFitterOpenMP.hh"
#include "MHO_SpectralLineFringeFitter.hh"

// //pybind11 stuff to interface with python
// #ifdef USE_PYBIND11
//     #include "pybind11_json/pybind11_json.hpp"
//     #include <pybind11/embed.h>
//     #include <pybind11/pybind11.h>
// namespace py = pybind11;
// namespace nl = nlohmann;
// using namespace pybind11::literals;
//     #include "MHO_PyConfigurePath.hh"
//     #include "MHO_PythonOperatorBuilder.hh"
// #endif

//julia/cxxwrap stuff to interface with julia
#ifdef HOPS_USE_JULIA
    #include "MHO_JuliaOperatorBuilder.hh"
#endif

namespace hops
{

MHO_FringeFitterFactory::MHO_FringeFitterFactory(MHO_FringeData* data): fFringeData(data), fFringeFitter(nullptr)
{}

MHO_FringeFitterFactory::~MHO_FringeFitterFactory()
{
    //delete the fringe fitter
    delete fFringeFitter;
}

MHO_FringeFitter* MHO_FringeFitterFactory::ConstructFringeFitter()
{
    //if it has already been built, just return the existing one
    if(fFringeFitter != nullptr)
    {
        return fFringeFitter;
    }

    //determine which fringe fitter to construct
    bool do_spectral_line = false;
    if(fFringeData->GetParameterStore()->IsPresent("/control/fit/spectral_line") )
    {
        fFringeData->GetParameterStore()->Get("/control/fit/spectral_line", do_spectral_line);
    }
    
    bool do_ion = false;
    fFringeData->GetParameterStore()->Get("/config/do_ion", do_ion);

    if(do_spectral_line)
    {
        msg_debug("fringe", "constructing a spectral line fringe fitter" << eom);
        fFringeFitter = new MHO_SpectralLineFringeFitter(fFringeData);
    }
    else if(do_ion)
    {
        msg_debug("fringe", "constructing an ionospheric fringe fitter" << eom);
        // #ifdef _OPENMP
        // fFringeFitter = new MHO_IonosphericFringeFitterOpenMP(fFringeData);
        // #else
        fFringeFitter = new MHO_IonosphericFringeFitter(fFringeData);
        // #endif
    }
    else
    {
        msg_debug("fringe", "constructing a basic fringe fitter" << eom);
        fFringeFitter = new MHO_BasicFringeFitter(fFringeData);
    }

    ////////////////////////////////////////////////////////////////////////////
    //POST-CONFIGURE FOR COMPILE-TIME EXTENSIONS
//     ////////////////////////////////////////////////////////////////////////////
// 
// #ifdef USE_PYBIND11
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_labeling", "python_labeling");
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_flagging", "python_flagging");
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_calibration",
//                                                                                           "python_calibration");
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_prefit", "python_prefit");
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_postfit", "python_postfit");
//     fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_PythonOperatorBuilder >("python_finalize", "python_finalize");
// #endif

#ifdef HOPS_USE_JULIA
    // Initialize the Julia runtime once per process before any Julia API calls.
    // jl_init() is not idempotent; guard with a static flag.
    static bool julia_runtime_initialized = false;
    if(!julia_runtime_initialized)
    {
        jl_init();
        // Do NOT register jl_atexit_hook via std::atexit.
        // In Julia 1.12+, running GC finalizers during C++ atexit crashes because
        // the JIT tries to compile finalizer methods for the first time while LLVM's
        // type registry is in a partially-torn-down state.  This also fires from any
        // std::exit() call (e.g. the interactive plot 'q' key), with no way to
        // guarantee a safe JIT state.  For a short-lived embedded Julia runtime the
        // OS reclaims all memory; explicit callers can call jl_atexit_hook(0) before
        // their own exit if a clean finalizer sweep is required.

        // Load HOPS CxxWrap modules.
        //
        // Containers -> loaded into Main via @wrapmodule / @initcxx.
        // Operators  -> @wrapmodule can only be called once per Julia module
        //              (it stores a guard in __cxxwrap_methodkeys).  Load the
        //              operators into a dedicated HOPSOps submodule instead,
        //              then import get_current_fringe_data into Main so that
        //              user Julia scripts can call it without qualification.
        //
        // CxxWrap.register_julia_module was removed in CxxWrap >= 0.15; this
        // approach works with all supported versions (0.15+).
        //
        // JULIA_MODULES_DIR is injected as a compile-time string by CMake.
        // Helper: evaluate one Julia statement, log+clear any exception, return false on failure.
        auto jl_eval_checked = [](const char* label, const char* code) -> bool {
            jl_eval_string(code);
            if(jl_exception_occurred())
            {
                msg_error("julia_bindings",
                          "Julia init step \"" << label << "\" failed: "
                              << jl_exception_message() << eom);
                jl_exception_clear();
                return false;
            }
            return true;
        };

        bool ok = true;
        ok &= jl_eval_checked("using CxxWrap", "using CxxWrap");
        ok &= jl_eval_checked("wrapmodule containers",
                              "@wrapmodule(() -> \"" JULIA_MODULES_DIR "/libjlMHO_Containers.so\")");
        ok &= jl_eval_checked("initcxx containers", "@initcxx");
        // Load operators into HOPSOps submodule (avoids the @wrapmodule guard on Main)
        ok &= jl_eval_checked("wrapmodule operators",
                              "module HOPSOps\n"
                              "  using CxxWrap\n"
                              "  @wrapmodule(() -> \"" JULIA_MODULES_DIR "/libjlMHO_Operators.so\")\n"
                              "  @initcxx\n"
                              "end");

        if(!ok)
            msg_error("julia_bindings", "HOPS Julia module initialization incomplete." << eom);

        julia_runtime_initialized = true;
    }
    fFringeFitter->GetOperatorBuildManager()->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_postfit", "julia_postfit");
#endif

    return fFringeFitter;
}

} // namespace hops
