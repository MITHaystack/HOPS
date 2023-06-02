#ifndef MHO_OperatorBuilder_HH__
#define MHO_OperatorBuilder_HH__

#include <string>
#include <utility>

#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops 
{


class MHO_OperatorBuilder
{

    public:
        MHO_OperatorBuilder():
            fName("")
        {};
        
        virtual ~MHO_OperatorBuilder(){}; //does not delete fOper, delegates memory management to downstream
        
        virtual void SetName(const std::string& name){fName = name;} //operator name

        //copy the json config
        virtual void SetConditions(const mho_json& cond){fConditions = cond;} //required conditions
        virtual void SetAttributes(const mho_json& attr){fAttributes = attr;}; //configuration parameters
        
        //how should we pass information about the arguments? //names? uuids? pointers?
        //this is tricky since we may not have all arguments available at the time of the operator's 
        //construction, need to think about this, for now leave this out.
        
        //builds the object and sets attributes, returns a nullptr if failed
        virtual std::pair<std::string, MHO_Operator*> Build() = 0;
        
    protected:

        std::string fName;
        mho_json fConditions;
        mho_json fAttributes;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
