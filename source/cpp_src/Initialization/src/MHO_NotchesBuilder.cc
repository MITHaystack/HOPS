#include "MHO_NotchesBuilder.hh"
#include "MHO_Notches.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_NotchesBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a notches operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "flagging";
        std::vector< double > values = fAttributes["value"].get< std::vector< double > >();
        double priority = fFormat["priority"].get< double >();

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_Notches without visibility data." << eom);
            return false;
        }

        //retrieve the arguments to operate on from the container store
        weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(wt_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_Notches without weight data." << eom);
            return false;
        }

        if(values.size() % 2 != 0)
        {
            msg_error("initialization", "cannot construct MHO_Notches, improper number ("
                                            << values.size() << "), of frequency limits given (multiple of 2 required)" << eom);
            return false;
        }

        MHO_Notches* op = new MHO_Notches();

        //set the arguments
        op->SetArgs(vis_data);
        op->SetWeights(wt_data);
        op->SetNotchBoundaries(values);
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization", "creating operator: " << op_name << " with " << values.size() << " notches" << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
