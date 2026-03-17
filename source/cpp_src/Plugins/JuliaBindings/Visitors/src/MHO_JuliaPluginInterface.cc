#include "MHO_JuliaPluginInterface.hh"

namespace hops
{

bool MHO_JuliaPluginInterface::fInitialized = false;

MHO_JuliaPluginInterface::MHO_JuliaPluginInterface()
{
    Initialize();
}

MHO_JuliaPluginInterface::~MHO_JuliaPluginInterface()
{
    Finalize();
}

void MHO_JuliaPluginInterface::Initialize()
{
    if(!fInitialized)
    {
        msg_debug("julia_bindings", "initializing julia plugin interface" << eom);

        jl_init(); //jl_init() is not idempotent; guard with a static flag.
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

        // Helper lambda: evaluate one Julia statement, log+clear any exception, return false on failure.
        auto jl_eval_checked = [](const char* label, const char* code) -> bool 
        {
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
        ok &= jl_eval_checked("wrapmodule containers", "@wrapmodule(() -> \"" JULIA_MODULES_DIR "/libjlMHO_Containers.so\")");
        ok &= jl_eval_checked("initcxx containers", "@initcxx");
        // Load operators into HOPSOps submodule (avoids the @wrapmodule guard on Main)
        ok &= jl_eval_checked("wrapmodule operators",
                              "module HOPSOps\n"
                              "  using CxxWrap\n"
                              "  @wrapmodule(() -> \"" JULIA_MODULES_DIR "/libjlMHO_Operators.so\")\n"
                              "  @initcxx\n"
                              "end");

        if(!ok)
        {
            msg_error("julia_bindings", "HOPS Julia module initialization incomplete." << eom);
        }

        fInitialized = ok;
    }
}


void MHO_JuliaPluginInterface::Finalize()
{
    if(fInitialized)
    {
        jl_atexit_hook(0); 
        fInitialized = false;
    }
}


void 
MHO_JuliaPluginInterface::Visit(MHO_FringeFitter* fitter)
{
    auto build_man = fitter->GetOperatorBuildManager();
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_labeling", "julia_labeling");
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_flagging", "julia_flagging");
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_calibration", "julia_calibration");
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_prefit", "julia_prefit");
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_postfit", "julia_postfit");
    build_man->AddBuilderType< MHO_JuliaOperatorBuilder >("julia_finalize", "julia_finalize");
}


}
