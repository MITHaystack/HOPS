#include "MHO_StationDelayCorrectionBuilder.hh"
#include "MHO_StationDelayCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_StationDelayCorrectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a manual per-pol delay correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double pc_delay_offset = fAttributes["value"].get< double >();
        double priority = fFormat["priority"].get< double >();

        std::string station_id = ExtractStationIdentifier();

        //grab the reference frequency from the parameter store
        double ref_freq = fParameterStore->GetAs< double >(std::string("/control/config/ref_freq"));

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_StationDelayCorrection without visibility data." << eom);
            return false;
        }

        MHO_StationDelayCorrection* op = new MHO_StationDelayCorrection();

        //set the arguments
        op->SetArgs(vis_data);
        op->SetReferenceFrequency(ref_freq);
        op->SetPCDelayOffset(pc_delay_offset);
        op->SetStationIdentifier(station_id);
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization",
                  "creating operator: " << op_name << " for station: " << station_id << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        return true;
    }
    return false;
}

std::string MHO_StationDelayCorrectionBuilder::ExtractStationIdentifier()
{
    std::string station_id = "??";
    std::vector< std::string > condition_values = fConditions["value"].get< std::vector< std::string > >();

    for(auto it = condition_values.begin(); it != condition_values.end(); it++)
    {
        //grab the first station ID in the 'if' statement
        //this is ok 99% of the time, but what about if there is statement like: 'if station X or station X'?
        //would then need to check that this station is a member of this pass too, and if not use the next
        if(*it == "station")
        {
            it++;
            if(it != condition_values.end())
            {
                station_id = *it;
                return station_id;
            }
        }
    }
    return station_id;
}

} // namespace hops
