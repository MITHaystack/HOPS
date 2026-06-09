#include "MHO_PluginVisitorFactory.hh"

#ifdef USE_MATPLOTPP
    #include "MHO_BasicPlotVisitor.hh"
#endif

// The in-process python plug-in operators are provided by MHO_PythonPluginInterface,
// which owns the embedded interpreter; it is only available in an embedded (pybind11) build.
#if defined(USE_EMBEDDED_PYTHON)
    #include "MHO_PythonPluginInterface.hh"
#endif

namespace hops
{

MHO_PluginVisitorFactory::MHO_PluginVisitorFactory()
    : fPluginsInitialized(false), fPlotInitialized(false), fOutputInitialized(false), fParameterStore(nullptr)
{}

MHO_PluginVisitorFactory::~MHO_PluginVisitorFactory()
{
    for(std::size_t i = 0; i < fPluginVisitors.size(); i++)
    {
        delete fPluginVisitors[i];
        fPluginVisitors[i] = nullptr;
    }
    fPluginVisitors.clear();
    fPluginsInitialized = false;

    //plot visitors are managed by their respective factory (do not delete here)
    fPlotVisitors.clear();
    fPlotInitialized = false;

    //output visitor are managed by their respective factory (do not delete here)
    fOutputVisitors.clear();
    fOutputInitialized = false;
}

void MHO_PluginVisitorFactory::GetPluginVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors)
{
    if(!fPluginsInitialized)
    {
        ConstructPlugins();
    }
    visitors.clear();
    visitors = fPluginVisitors;
}

void MHO_PluginVisitorFactory::GetPlotVisitors(std::vector< MHO_FringePlotVisitor* >& visitors)
{
    if(!fPlotInitialized)
    {
        ConstructPlotters();
    }
    visitors.clear();
    visitors = fPlotVisitors;
}

void MHO_PluginVisitorFactory::GetOutputVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors)
{
    if(!fOutputInitialized)
    {
        ConstructOutputVisitors();
    }
    visitors.clear();
    visitors = fOutputVisitors;
}

void MHO_PluginVisitorFactory::ConstructPlugins()
{
    if(fParameterStore != nullptr)
    {

#if defined(USE_EMBEDDED_PYTHON)
        bool need_python_plugin = false;
        if(fParameterStore->IsPresent("/config/plugins/activate_python"))
        {
            need_python_plugin |= fParameterStore->GetAs< bool >("/config/plugins/activate_python");
        }

        std::string plot_backend;
        fParameterStore->Get("/control/config/plot_backend", plot_backend);
        if(plot_backend == "matplotlib") //embedded matplotlib plotting needs the in-process interpreter
        {
            need_python_plugin |= true;
        }

        if(need_python_plugin)
        {
            msg_debug("plugins", "constructing the python plugin interface" << eom);
            MHO_FringeFitterVisitor* py_visitor = new MHO_PythonPluginInterface();
            fPluginVisitors.push_back(py_visitor);
        }

#else
        // No embedded interpreter in this build. The default plotter and Python
        // control files still work via the subprocess backend, but in-process
        // user plug-in operators (zero-copy access to the data) are unavailable.
        // Emit a clear error if the control file explicitly requests them rather
        // than letting it fail later as an unknown operator.
        bool activate_python = false;
        if(fParameterStore->IsPresent("/config/plugins/activate_python"))
        {
            activate_python = fParameterStore->GetAs< bool >("/config/plugins/activate_python");
        }
        if(activate_python)
        {
            msg_error("plugins", "Python plug-in operators require HOPS to be built against an embedded "
                                 "Python interpreter (HOPS_ENABLE_EMBEDDED_PYTHON=ON), they are unavailable in this "
                                 "binary and will be ignored"
                                     << eom);
        }
#endif
    }
    fPluginsInitialized = true;
}

void MHO_PluginVisitorFactory::ConstructPlotters()
{
    if(fParameterStore != nullptr)
    {
        //currently we only have two fringe plotting options (gnuplot or matplotlib)
        std::string plot_backend;
        fParameterStore->Get("/control/config/plot_backend", plot_backend);
        MHO_FringePlotVisitor* plotter = fPlotterFactory.ConstructPlotter(plot_backend);
        if(plotter)
        {
            msg_debug("plugin", "plot factory is adding a plotter with the backend: " << plot_backend << eom);
            fPlotVisitors.push_back(plotter);
        }
    }
    fPlotInitialized = true;
}

void MHO_PluginVisitorFactory::ConstructOutputVisitors()
{
    //currently, we assume only one format is requested at a time
    //however, there is no limitation to having multiple formats generated at the same time if desired

    std::string output_format = "hops4";
    if(fParameterStore != nullptr)
    {
        bool use_mk4_output = false;
        fParameterStore->Get("/cmdline/mk4format_output", use_mk4_output);
        if(use_mk4_output)
        {
            output_format = "mark4";
        }

        MHO_FringeFitterVisitor* output_visitor = nullptr;
        output_visitor = fOutputFactory.GetOutputVisitor(output_format);

        if(output_visitor)
        {
            msg_debug("plugin", "output factory is adding format: " << output_format << eom);
            fOutputVisitors.push_back(output_visitor);
        }
    }
    fOutputInitialized = true;
}

} // namespace hops
