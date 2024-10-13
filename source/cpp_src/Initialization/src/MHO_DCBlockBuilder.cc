#include "MHO_DCBlockBuilder.hh"
#include "MHO_DCBlock.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_DCBlockBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a dc_block operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "flagging";
        bool value = fAttributes["value"].get< bool >();
        double priority = fFormat["priority"].get< double >();

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_DCBlock without visibility data." << eom);
            return false;
        }

        if(value)
        {
            MHO_DCBlock* op = new MHO_DCBlock();
            //set the arguments
            op->SetArgs(vis_data);
            op->SetName(op_name);
            op->SetPriority(priority);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        }

        return true;
    }
    return false;
}

} // namespace hops
