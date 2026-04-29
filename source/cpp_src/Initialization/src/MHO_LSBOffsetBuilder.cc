#include "MHO_LSBOffsetBuilder.hh"
#include "MHO_LSBOffset.hh"

#include <memory>

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_LSBOffsetBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building lsb_offset phase correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double lsb_phase_offset = fAttributes["value"].get< double >();
        double priority = fFormat["priority"].get< double >();

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_LSBOffset without visibility data." << eom);
            return false;
        }

        std::unique_ptr< MHO_LSBOffset > op(new MHO_LSBOffset());

        //set the arguments
        op->SetArgs(vis_data);
        op->SetLSBPhaseOffset(lsb_phase_offset);
        op->SetStationIdentifiers(ExtractAllStationIdentifiers());
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization", "creating operator: " << op_name << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(std::move(op), op_name, op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
