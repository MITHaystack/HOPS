#ifndef MHO_PluginVisitorFactory_HH__
#define MHO_PluginVisitorFactory_HH__

#include <vector>

#include "MHO_Message.hh"
#include "MHO_FringeFitter.hh"
#include "MHO_FringePlotVisitor.hh"
#include "MHO_FringePlotVisitorFactory.hh"

namespace hops
{


class MHO_PluginVisitorFactory
{
    public:
        MHO_PluginVisitorFactory();
        virtual ~MHO_PluginVisitorFactory();
        
        void SetParameterStore(MHO_ParameterStore* params){fParameterStore = params;}

        //plugin visitors
        void GetPluginVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors);

        //plot visitors
        void GetPlotVisitors(std::vector< MHO_FringePlotVisitor* >& visitors);

        //(file) output visitors
        void GetOutputVisitors(std::vector< MHO_FringeFitterVisitor*>& visitors);

    protected:
        
        void ConstructPlugins();
        void ConstructPlotters();
        void ConstructOutputVisitors();

        bool fPluginsInitialized;
        bool fPlotInitialized;
        bool fOutputInitialized;
        MHO_ParameterStore* fParameterStore;

        std::vector< MHO_FringeFitterVisitor* > fPluginVisitors;
        std::vector< MHO_FringeFitterVisitor* > fOutputVisitors;

        std::vector< MHO_FringePlotVisitor* > fPlotVisitors;
        MHO_FringePlotVisitorFactory fPlotterFactory;
};


} // namespace hops

#endif /*! end of include guard: MHO_PluginVisitorFactory_HH__ */
