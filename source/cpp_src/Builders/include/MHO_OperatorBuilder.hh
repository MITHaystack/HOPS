#ifndef MHO_OperatorBuilder_HH__
#define MHO_OperatorBuilder_HH__

#include <string>
#include <utility>

#include "MHO_Message.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Operator.hh"

namespace hops 
{


class MHO_OperatorBuilder
{

    public:
        MHO_OperatorBuilder():
            fName(""),
            fOper(nullptr)
        {};
        
        virtual ~MHO_OperatorBuilder(){}; //does not delete fOper, delegates memory management to downstream
        
        virtual void SetName(const std::string& name){fName = name;} //operator name
        virtual void SetAttributes(const mho_json& attr){fAttr = attr;}; //configuration parameters
        
        //how should we pass information about the arguments? //names? uuids? pointers?
        //this is tricky since we may not have all arguments available at the time of the operator's 
        //construction, need to think about this
        
        //builds the object and sets attributes, returns true iff successful
        //returns false if it fails and deletes any allocated memory
        virtual bool Build() = 0;
        
        //return a name, ptr pair so we can insert this operator in a map
        virtual std::pair<std::string, MHO_Operator*> GetNamedOperator()
        {
            return std::make_pair(fName, fOper);
        }
        
    private:

        std::string fName;
        const mho_json& fAttr;
        MHO_Operator* fOper;
};


}

#endif /* end of include guard: MHO_OperatorBuilder_HH__ */
