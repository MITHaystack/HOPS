#include "MHO_SubprocessPythonPlotVisitor.hh"

#include "MHO_FringeData.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_PythonSubprocessContract.hh"
#include "MHO_PythonSubprocessRunner.hh"

namespace hops
{

void MHO_SubprocessPythonPlotVisitor::Plot(MHO_FringeData* data)
{
    namespace pc = python_subprocess;

    MHO_ParameterStore* params = data->GetParameterStore();

    //match the embedded path: do not plot a skipped pass
    bool is_skipped = params->GetAs< bool >("/status/skipped");
    if(is_skipped)
    {
        return;
    }

    //custom plotting is embedded-only; warn and render the default plot
    if(params->IsPresent("/control/config/python_custom_plot"))
    {
        msg_warn("python_subprocess", "custom Python plotting requires HOPS to be built with embedded Python "
                                      "(HOPS_ENABLE_EMBEDDED_PYTHON=ON), rendering the default plot instead"
                                          << eom);
    }

    //assemble the request
    mho_json request;
    request[pc::kSchemaVersionKey] = pc::kSchemaVersion;
    request[pc::plot::kPlotDictKey] = data->GetPlotData();

    bool show_plot = false;
    if(params->IsPresent("/cmdline/show_plot"))
    {
        params->Get("/cmdline/show_plot", show_plot);
    }
    request[pc::plot::kShowPlotKey] = show_plot;

    std::string disk_file;
    if(params->IsPresent("/cmdline/disk_file"))
    {
        params->Get("/cmdline/disk_file", disk_file);
    }
    request[pc::plot::kDiskFileKey] = disk_file;

    msg_debug("python_subprocess", "rendering fringe plot via python subprocess" << eom);

    MHO_PythonSubprocessRunner::Result result = MHO_PythonSubprocessRunner::RunModule(pc::plot::kModule, request.dump());

    //FAILURE BEHAVIOR: warn and skip the plot, then continue.
    if(!result.spawned)
    {
        msg_warn("python_subprocess", "could not launch python3 to render the plot; skipping plot" << eom);
        return;
    }

    bool ok = false;
    std::string err_msg;
    try
    {
        mho_json response = mho_json::parse(result.out);
        ok = response.value(pc::kOkKey, false);
        if(!ok)
        {
            err_msg = response.value(pc::kErrorKey, std::string("unknown error"));
        }
    }
    catch(const std::exception& e)
    {
        ok = false;
        err_msg = std::string("could not parse plot subprocess response: ") + e.what();
    }

    if(!ok || !result.Success())
    {
        if(err_msg.empty())
        {
            err_msg = "python plot subprocess exited abnormally";
        }
        msg_warn("python_subprocess", "python plot rendering failed; skipping plot: " << err_msg << eom);
        if(!result.err.empty())
        {
            msg_warn("python_subprocess", "python stderr: " << result.err << eom);
        }
        return;
    }

    msg_debug("python_subprocess", "python plot rendering complete" << eom);
}

} // namespace hops
