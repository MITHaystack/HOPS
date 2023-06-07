#ifndef MHO_OperatorBuilder_HH__
#define MHO_OperatorBuilder_HH__

#include <string>
#include <utility>

#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{


class MHO_OperatorBuilder
{

    public:

        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox):
            fOperatorToolbox(toolbox),
            fContainerStore(nullptr),
            fParameterStore(nullptr)
            {};
            
        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore, mho_json* pstore):
            fOperatorToolbox(toolbox),
            fContainerStore(cstore),
            fParameterStore(pstore)
            {};
            
            
        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox
        
        virtual void SetParameterStore(mho_json* pstore){fParameterStore = pstore;} 
        virtual void SetContainerStore(MHO_ContainerStore* cstore){fContainerStore = cstore;}

        //json config for this operator (parsed from the control file)
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //conditional statements
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters

        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false and operator is not constructed
        virtual bool Build() = 0;

    protected:
        
        //constructed operator gets stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //container store and data look-up library (for retrieval and setting of arguments)
        MHO_ContainerStore* fContainerStore;
        mho_json* fParameterStore;

        //provided for the configuration of the operator that is to be built
        mho_json fConditions;
        mho_json fAttributes;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
