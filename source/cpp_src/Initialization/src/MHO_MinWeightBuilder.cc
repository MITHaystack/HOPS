#include "MHO_MinWeightBuilder.hh"
#include "MHO_MinWeight.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_MinWeightBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building a min_weight operator."<< eom);
        //assume attributes are ok for now - TODO add checks!
        
        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "selection";
        double value = fAttributes["value"].get< double >();
        double priority = fFormat["priority"].get<double>();

        //retrieve the arguments to operate on from the container store
        weight_type* wt_data = fContainerStore->GetObject<weight_type>(std::string("weight"));
        if( wt_data == nullptr )
        {
            msg_error("initialization", "cannot construct MHO_MinWeight without visibility data." << eom);
            return false;
        }

        if(value)
        {
            MHO_MinWeight* op = new MHO_MinWeight();
            //set the arguments
            op->SetArgs(wt_data);
            op->SetMinWeight(value);
            op->SetName(op_name);
            op->SetPriority(priority);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
        }
           
        return true;

    }
    return false;
}





}//end namespace
