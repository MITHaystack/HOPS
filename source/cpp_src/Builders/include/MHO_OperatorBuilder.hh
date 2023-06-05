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

        MHO_OperatorBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* store):
            fOperatorToolbox(toolbox),
            fContainerStore(store)
            {};
            
        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox
        
        //json description of the data library for this pass
        //the library maps argument names to object UUID
        //which can then be used to be retrieve an object from the scan data store
        virtual void SetDataLibrary(const mho_json& lib){fDataLibrary = lib;} 

        //json config for this operator (parse from the control file)
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //required conditions
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters

        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false
        virtual bool Build() = 0;

    protected:
        
        MHO_OperatorToolbox* fOperatorToolbox;
        MHO_ContainerStore* fContainerStore;

        mho_json fDataLibrary;
        mho_json fConditions;
        mho_json fAttributes;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
