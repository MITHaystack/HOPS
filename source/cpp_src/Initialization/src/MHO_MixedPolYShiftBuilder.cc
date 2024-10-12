#include "MHO_MixedPolYShiftBuilder.hh"
#include "MHO_MixedPolYShift.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_MixedPolYShiftBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building mixed_pol_yshift90 correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double priority = fFormat["priority"].get< double >();

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_MixedPolYShift without visibility data." << eom);
            return false;
        }

        MHO_MixedPolYShift* op = new MHO_MixedPolYShift();

        //set the arguments
        op->SetArgs(vis_data);
        op->SetName(op_name);
        op->SetPriority(priority);

        bool replace_duplicates = true; //only one operator of this type is needed
        this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
