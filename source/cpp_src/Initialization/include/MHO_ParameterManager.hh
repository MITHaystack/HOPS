#ifndef MHO_ParameterManager_HH__
#define MHO_ParameterManager_HH__

#include "MHO_ParameterConfigurator.hh"

namespace hops
{
    
class MHO_ParameterManager
{
    public:
        
        MHO_ParameterManager(MHO_ParameterStore* pstore, mho_json control_format):
            fDefaultParameterConfig(pstore,control_format)
        {};

        virtual ~MHO_ParameterManager(){};

        void SetControlStatements(const mho_json& statements){fControl = statements;};
        
        void ConfigureAll();

    private:
        
        
        mho_json fControl;

        MHO_ParameterConfigurator fDefaultParameterConfig;
};

}


#endif /* end of include guard: MHO_ParameterManager_HH__ */
