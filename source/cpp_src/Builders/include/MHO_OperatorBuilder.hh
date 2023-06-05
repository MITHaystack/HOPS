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
            fDataLibrary(nullptr)
            {};
            
        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox
        
        //json description of the data library for this pass
        //the library maps argument names to an object UUID
        //which can then be used to be retrieve an object from the scan data store
        virtual void SetDataLibrary(mho_json* lib){fDataLibrary = lib;} 
        virtual void SetContainerStore(MHO_ContainerStore* store){fContainerStore = store;}

        //json config for this operator (parse from the control file)
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //conditional statements
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters

        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false
        virtual bool Build() = 0;

    protected:
        
        //constructed operator gets stashed here
        MHO_OperatorToolbox* fOperatorToolbox;

        //container store and data look-up library (for retrieval and setting of arguments)
        MHO_ContainerStore* fContainerStore;
        mho_json* fDataLibrary;

        //provided for the configuration of the operator that is to be built
        mho_json fConditions;
        mho_json fAttributes;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
