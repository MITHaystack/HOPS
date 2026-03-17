#include "MHO_PluginVisitorFactory.hh"

#ifdef USE_MATPLOTPP
    #include "MHO_BasicPlotVisitor.hh"
#endif

#ifdef USE_PYBIND11
    #include "MHO_PythonPluginInterface.hh"
#endif

#ifdef HOPS_USE_JULIA
    #include "MHO_JuliaPluginInterface.hh"
#endif

namespace hops
{
    
    
MHO_PluginVisitorFactory::MHO_PluginVisitorFactory():
    fInitialized(false),
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
    fInitialized = false;
}


void MHO_PluginVisitorFactory::GetPluginVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors)
{
    if(!fInitialized){ConstructPlugins();}
    visitors.clear();
    visitors = fPluginVisitors;
}


void MHO_PluginVisitorFactory::ConstructPlugins()
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
        if( fParameterStore->IsPresent("/config/plugins/activate_python") )
        {
            bool need_python_plugin = fParameterStore->GetAs<bool>("/config/plugins/activate_python");
            if(need_python_plugin)
            {
                MHO_FringeFitterVisitor* py_visitor = new MHO_PythonPluginInterface();
                fPluginVisitors.push_back(py_visitor);
            }
        }
        #endif 
    }
}

} // namespace hops
