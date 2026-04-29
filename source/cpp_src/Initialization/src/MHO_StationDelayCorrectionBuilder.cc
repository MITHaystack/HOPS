#include "MHO_StationDelayCorrectionBuilder.hh"
#include "MHO_StationDelayCorrection.hh"

#include <memory>

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_StationDelayCorrectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a manual per-station delay correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double pc_delay_offset = fAttributes["value"].get< double >(); //assumption is nano seconds
        double priority = fFormat["priority"].get< double >();

        //grab the reference frequency from the parameter store
        double ref_freq = fParameterStore->GetAs< double >(std::string("/control/config/ref_freq"));

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_StationDelayCorrection without visibility data." << eom);
            return false;
        }

        std::unique_ptr< MHO_StationDelayCorrection > op(new MHO_StationDelayCorrection());

        //set the arguments
        op->SetArgs(vis_data);
        op->SetReferenceFrequency(ref_freq);
        op->SetPCDelayOffset(pc_delay_offset);
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
