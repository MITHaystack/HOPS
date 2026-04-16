#include "MHO_FringePass.hh"

#include <memory>

#include "MHO_BasicFringeFitter.hh"
#include "MHO_ControlDefinitions.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_FringeControlInitialization.hh"
#include "MHO_FringeDataInitializer.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_IonosphericFringeFitter.hh"
#include "MHO_SpectralLineFringeFitter.hh"

namespace hops
{

MHO_FringePass::MHO_FringePass() : fFactory(nullptr) {}

// ---------------------------------------------------------------------------
// Input setters
// ---------------------------------------------------------------------------

void MHO_FringePass::CopyCommandLineParams(const MHO_ParameterStore& cmdline_params)
{
    fData.GetParameterStore()->CopyFrom(cmdline_params);
}

void MHO_FringePass::SetScanDirectory(const std::string& dir)
{
    fData.GetParameterStore()->Set("/pass/input_directory", dir);
}

void MHO_FringePass::SetBaseline(const std::string& baseline)
{
    fData.GetParameterStore()->Set("/pass/baseline", baseline);
}

void MHO_FringePass::SetPolProduct(const std::string& polprod)
{
    fData.GetParameterStore()->Set("/pass/polprod", polprod);
}

void MHO_FringePass::SetFrequencyGroup(const std::string& fgroup)
{
    fData.GetParameterStore()->Set("/pass/frequency_group", fgroup);
}

void MHO_FringePass::SetScanName(const std::string& scan)
{
    fData.GetParameterStore()->Set("/pass/scan", scan);
}

void MHO_FringePass::SetRootFile(const std::string& root_file)
{
    fData.GetParameterStore()->Set("/pass/root_file", root_file);
}

void MHO_FringePass::SetBuildTimestamp(const std::string& ts)
{
    fData.GetParameterStore()->Set("/pass/build_time", ts);
}

void MHO_FringePass::SetControlFile(const std::string& path)
{
    fControlFileOverride = path;
}

void MHO_FringePass::SetPythonControlEvaluator(ControlEvaluatorFn fn)
{
    fPythonEvaluator = fn;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool MHO_FringePass::Initialize()
{
    profiler_scope();
    bool ok = MHO_FringeDataInitializer::initialize_scan_data(fData.GetParameterStore(),
                                                              fData.GetScanDataStore());
    if(!ok)
    {
        return false;
    }

    MHO_FringeDataInitializer::populate_initial_parameters(fData.GetParameterStore(),
                                                           fData.GetScanDataStore());

    // If a control file was set explicitly, override what populate_initial_parameters
    // copied from /cmdline/control_file.
    if(!fControlFileOverride.empty())
    {
        fData.GetParameterStore()->Set("/files/control_file", fControlFileOverride);
    }

    return !IsSkipped();
}

bool MHO_FringePass::Configure()
{
    profiler_scope();
    std::string ctrl_file = fData.GetParameterStore()->GetAs< std::string >("/files/control_file");
    std::string ext = MHO_DirectoryInterface::GetFileExtension(ctrl_file);

    if(ext == "py")
    {
        if(!fPythonEvaluator)
        {
            msg_fatal("fringe_pass",
                      "Python control file '" << ctrl_file
                          << "' specified but no Python evaluator was injected,  "
                          << " pybind11 is required for this feature" << eom);
            return false;
        }

        fData.GetParameterStore()->Set("/status/is_finished", false);
        fData.GetParameterStore()->Set("/status/skipped", false);

        // Populate control format - same setup as the DSL path.
        fData.GetControlFormat() = MHO_ControlDefinitions::GetControlFormat();
        MHO_FringeControlInitialization::add_default_operator_format_def(fData.GetControlFormat());

        bool py_ok = fPythonEvaluator(fData.GetParameterStore(),
                                      fData.GetControlFormat(),
                                      fData.GetControlStatements());
        if(!py_ok)
        {
            msg_error("fringe_pass", "Python control file evaluation failed, skipping pass." << eom);
            fData.GetParameterStore()->Set("/status/skipped", true);
            fData.GetParameterStore()->Set("/status/is_finished", true);
            return false;
        }

        MHO_FringeControlInitialization::apply_control_statements(fData.GetParameterStore(),
                                                                  fData.GetControlFormat(),
                                                                  fData.GetControlStatements());

        // A Python control file implicitly activates the Python plugin.
        fData.GetParameterStore()->Set("/config/plugins/activate_python", true);
    }
    else
    {
        // DSL path
        MHO_FringeControlInitialization::process_control_file(fData.GetParameterStore(),
                                                              fData.GetControlFormat(),
                                                              fData.GetControlStatements());
    }

    return !IsSkipped();
}

bool MHO_FringePass::Run(const std::vector< MHO_FringeFitterVisitor* >& plugin_visitors,
                         const std::vector< MHO_FringeFitterVisitor* >& output_visitors,
                         const std::vector< MHO_FringePlotVisitor* >&   plot_visitors)
{
    //use explicit profiler start/stop for this function, because we need to export events
    //before the output is written
    profiler_start();

    // Build the appropriate fringe fitter (basic, ionospheric, or spectral-line).
    // fFactory owns the fitter for the lifetime of this MHO_FringePass.
    fFactory.reset(new MHO_FringeFitterFactory(&fData));
    MHO_FringeFitter* ffit = fFactory->ConstructFringeFitter();

    if(ffit == nullptr)
    {
        msg_error("fringe_pass", "failed to construct a fringe fitter, skipping pass." << eom);
        return false;
    }

    // Register plugin visitors before Configure() - required ordering.
    for(auto* v : plugin_visitors)
    {
        ffit->Accept(v);
    }

    ffit->Configure();

    while(!ffit->IsFinished())
    {
        ffit->Initialize();
        ffit->PreRun();
        ffit->Run();
        ffit->PostRun();
    }
    ffit->Finalize();

    //flush profile events (must be done before output is written)
    profiler_stop();
    FlushProfileEvents();

    if(IsSkipped())
    {
        return true;
    }

    // Dispatch post-run visitors (caller passes empty vectors to suppress).
    for(auto* v : output_visitors)
    {
        ffit->Accept(v);
    }
    for(auto* v : plot_visitors)
    {
        ffit->Accept(v);
    }

    return true;
}

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------

bool MHO_FringePass::IsSkipped()
{
    bool skipped = false;
    fData.GetParameterStore()->Get("/status/skipped", skipped);
    return skipped;
}

void MHO_FringePass::FlushProfileEvents()
{
    std::vector< MHO_ProfileEvent > events;
    MHO_Profiler::GetInstance().GetEvents(events);
    mho_json event_list;
    for(std::size_t i = 0; i < events.size(); i++)
    {
        mho_json obj;
        obj["event_id"] = i;
        obj["flag"]      = events[i].fFlag;
        obj["line"]      = events[i].fLineNumber;
        obj["thread_id"] = events[i].fThreadID;
        obj["filename"]  = std::string(events[i].fFilename);
        obj["funcname"]  = std::string(events[i].fFuncname);
        obj["time"]      = events[i].fTime;
        event_list.push_back(obj);
    }
    fData.GetParameterStore()->Set("/profile/events", event_list);
}

} // namespace hops
