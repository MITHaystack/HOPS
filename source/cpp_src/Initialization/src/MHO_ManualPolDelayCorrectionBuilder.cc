#include "MHO_ManualPolDelayCorrectionBuilder.hh"
#include "MHO_ManualPolDelayCorrection.hh"

#include <memory>

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_ManualPolDelayCorrectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a manual per-pol delay correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double pc_delay_offset = fAttributes["value"].get< double >();
        double priority = fFormat["priority"].get< double >();

        std::string pol = ParsePolFromName(op_name);

        //TODO FIXME...this is up for debate, should we:
        //(1) Make each operator name station specific (so toolbox retrieval can grab them individually by station)
        //or (2) Leave the operator name purely as a 'type' specifier, and have the toolbox return a list of ops
        //of the same time. The user/caller then needs to query each operator to find the specific one.
        //Note that (1) doesn't necessarily guarantee operator name uniqueness
        // uncomment the following for (1):
        // op_name += ".";
        // op_name += station_id;

        //grab the reference frequency from the parameter store
        double ref_freq = fParameterStore->GetAs< double >(std::string("/control/config/ref_freq"));

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_ManualPolDelayCorrection without visibility data." << eom);
            return false;
        }

        std::unique_ptr< MHO_ManualPolDelayCorrection > op(new MHO_ManualPolDelayCorrection());

      //set the arguments
        op->SetArgs(vis_data);
        op->SetReferenceFrequency(ref_freq);
        op->SetPCDelayOffset(pc_delay_offset);
        op->SetPolarization(pol);
        op->SetStationIdentifiers(GetMatchingStationIdentifiers());
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization", "creating operator: " << op_name << " pol: " << pol << "." << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(std::move(op), op_name, op_category, replace_duplicates);
        return true;
    }
    return false;
}

std::string MHO_ManualPolDelayCorrectionBuilder::ParsePolFromName(const std::string& name)
{
    if(name == "pc_delay_x")
    {
        return std::string("X");
    }
    if(name == "pc_delay_y")
    {
        return std::string("Y");
    }
    if(name == "pc_delay_r")
    {
        return std::string("R");
    }
    if(name == "pc_delay_l")
    {
        return std::string("L");
    }
    return std::string("?");
}

} // namespace hops
