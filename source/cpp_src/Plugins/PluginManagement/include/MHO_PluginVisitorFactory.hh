#ifndef MHO_PluginVisitorFactory_HH__
#define MHO_PluginVisitorFactory_HH__

#include <vector>

#include "MHO_Message.hh"
#include "MHO_FringeFitter.hh"

namespace hops
{


class MHO_PluginVisitorFactory
{
    public:
        MHO_PluginVisitorFactory();
        virtual ~MHO_PluginVisitorFactory();
        
        void SetParameterStore(MHO_ParameterStore* params){fParameterStore = params;}
        void GetPluginVisitors(std::vector< MHO_FringeFitterVisitor* >& visitors);

    protected:
        
        void ConstructPlugins();

        bool fInitialized;
        MHO_ParameterStore* fParameterStore;
        std::vector< MHO_FringeFitterVisitor* > fPluginVisitors;
};


} // namespace hops

#endif /*! end of include guard: MHO_PluginVisitorFactory_HH__ */
