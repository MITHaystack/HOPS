#include "MHO_PluginVisitorFactory.hh"

#ifdef USE_MATPLOTPP
    #include "MHO_BasicPlotVisitor.hh"
#endif

#ifdef USE_PYBIND11
    #include "MHO_PythonPluginInterface.hh"
    #include "MHO_DefaultPythonPlotVisitor.hh"
#endif

#ifdef HOPS_USE_JULIA
    #include "MHO_JuliaPluginInterface.hh"
#endif

namespace hops
{
    
    
MHO_PluginVisitorFactory::MHO_PluginVisitorFactory():
    fPluginsInitialized(false),
    fPlotInitialized(false),
    fOutputInitialized(false),
    fParameterStore(nullptr)
{}

MHO_PluginVisitorFactory::~MHO_PluginVisitorFactory()
{
    for(std::size_t i = 0; i<fPluginVisitors.size(); i++)
    {
        delete fPluginVisitors[i];
        fPluginVisitors[i] = nullptr;
    }
    fPluginVisitors.clear();
    fPluginsInitialized = false;

    for(std::size_t i = 0; i<fPlotVisitors.size(); i++)
    {
        delete fPlotVisitors[i];
        fPlotVisitors[i] = nullptr;
    }
    fPlotVisitors.clear();
    fPlotInitialized = false;

    for(std::size_t i = 0; i<fOutputVisitors.size(); i++)
    {
        delete fOutputVisitors[i];
        fOutputVisitors[i] = nullptr;
    }
    fOutputVisitors.clear();
    fOutputInitialized = false;
}

void 
MHO_PluginVisitorFactory::GetPluginVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors)
{
    if(!fPluginsInitialized){ConstructPlugins();}
    visitors.clear();
    visitors = fPluginVisitors;
}


void 
MHO_PluginVisitorFactory::GetPlotVisitors(std::vector< MHO_FringePlotVisitor* >& visitors)
{
    if(!fPlotInitialized){ConstructPlotters();}
    visitors.clear();
    visitors = fPlotVisitors;
}


void 
MHO_PluginVisitorFactory::GetOutputVisitors(std::vector< MHO_FringeFitterVisitor*>& visitors)
{
    if(!fOutputInitialized){ConstructOutputVisitors();}
    visitors.clear();
    visitors = fOutputVisitors;
}


void 
MHO_PluginVisitorFactory::ConstructPlugins()
{
    if(fParameterStore != nullptr)
    {
        #ifdef HOPS_USE_JULIA
        if( fParameterStore->IsPresent("/config/plugins/activate_julia") )
        {
            bool need_julia_plugin = fParameterStore->GetAs<bool>("/config/plugins/activate_julia");
            if(need_julia_plugin)
            {
                MHO_FringeFitterVisitor* jl_visitor = new MHO_JuliaPluginInterface();
                fPluginVisitors.push_back(jl_visitor);
            }
        }
        #endif 
        
        
        #ifdef USE_PYBIND11
        bool need_python_plugin = false;
        if( fParameterStore->IsPresent("/config/plugins/activate_python") )
        {
            need_python_plugin |= fParameterStore->GetAs<bool>("/config/plugins/activate_python");
        }

        std::string plot_backend;
        fParameterStore->Get("/control/config/plot_backend", plot_backend);
        if(plot_backend == "matplotlib") //we need the python plugin if the plotter is matplotlib too
        {
            need_python_plugin |= true;
        }

        if(need_python_plugin)
        {
            MHO_FringeFitterVisitor* py_visitor = new MHO_PythonPluginInterface();
            fPluginVisitors.push_back(py_visitor);
        }

        #endif 
    }
}


void 
MHO_PluginVisitorFactory::ConstructPlotters()
{
    if(fParameterStore != nullptr)
    {
        //currently we only have two fringe plotting options (gnuplot or matplotlib)
        std::string plot_backend;
        fParameterStore->Get("/control/config/plot_backend", plot_backend);
        MHO_FringePlotVisitor* plotter = fPlotterFactory.ConstructPlotter(plot_backend);
        if(plotter){fPlotVisitors.push_back(plotter);}
    }
}


void 
MHO_PluginVisitorFactory::ConstructOutputVisitors()
{

}









} // namespace hops
