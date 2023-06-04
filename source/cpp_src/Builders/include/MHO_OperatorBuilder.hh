#ifndef MHO_OperatorBuilder_HH__
#define MHO_OperatorBuilder_HH__

#include <string>
#include <utility>

#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{


class MHO_OperatorBuilder
{

    public:

        MHO_OperatorBuilder(){};
        virtual ~MHO_OperatorBuilder(){}; //delegate memory management to toolbox

        //copy the json config for this operator
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //required conditions
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters
        
        //how should we pass information about the arguments? //names? uuids? pointers?
        //this is tricky since we may not have all arguments available at the time of the operator's 
        //construction, need to think about this, for now leave this out.
        
        //builds the object, if successful passes to toolbox and returns true
        //otherwise returns false
        virtual bool Build(const mho_json& cond, const mho_json& attr)
        {
            fConditions = cond;
            fAttributes = attr;
            return BuildImpl();
        }

    protected:

        //derived class implementation
        virtual bool BuildImpl() = 0;

        mho_json fConditions;
        mho_json fAttributes;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
