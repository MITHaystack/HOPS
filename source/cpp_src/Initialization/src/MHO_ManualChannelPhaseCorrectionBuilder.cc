#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_ManualChannelPhaseCorrectionBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building a manual per-channel phase correction operator."<< eom);
        //assume attributes are ok for now - TODO add checks!
        #pragma message("TODO FIXME -- add checks on attributes (e.g. IsConfigurationOk)" )

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "calibration";

        std::string channel_name_str = fAttributes["value"]["channel_names"].get<std::string>();
        std::vector<double> pc_phases = fAttributes["value"]["pc_phases"].get< std::vector<double> >();
        //construct channel -> pc_phase map
        auto chan2pcp = MapChannelQuantities(channel_name_str, pc_phases);

        double priority = fFormat["priority"].get<double>();

        std::string pol = ParsePolFromName(op_name);
        std::string mk4id = ExtractStationMk4ID();


        if( chan2pcp.size() > 0)
        {
            //retrieve the arguments to operate on from the container store
            visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
            if( vis_data == nullptr )
            {
                msg_error("initialization", "cannot construct MHO_ManualChannelPhaseCorrection without visibility data." << eom);
                return false;
            }

            MHO_ManualChannelPhaseCorrection* op = new MHO_ManualChannelPhaseCorrection();

            //set the arguments
            op->SetArgs(vis_data);
            op->SetChannelToPCPhaseMap(chan2pcp);
            op->SetPolarization(pol);
            op->SetStationMk4ID(mk4id);
            op->SetName(op_name);
            op->SetPriority(priority);

            msg_debug("initialization", "creating operator: "<<op_name<<" for station: "<<mk4id<<" pol: "<<pol<<"."<<eom);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
            return true;
        }
        else
        {
            msg_error("initialization", "cannot set pc_phases with unequal number of channels/elements. " << eom);
            return false;
        }
    }
    return false;
}

std::string
MHO_ManualChannelPhaseCorrectionBuilder::ParsePolFromName(const std::string& name)
{
    if(name == "pc_phases_x"){return std::string("X");}
    if(name == "pc_phases_y"){return std::string("Y");}
    if(name == "pc_phases_r"){return std::string("R");}
    if(name == "pc_phases_l"){return std::string("L");}
    return std::string("?");
}


std::string
MHO_ManualChannelPhaseCorrectionBuilder::ExtractStationMk4ID()
{
    std::string station_id = "?";
    std::vector< std::string > condition_values = fConditions["value"].get< std::vector< std::string > >();

    for(auto it = condition_values.begin(); it != condition_values.end(); it++)
    {
         //grab the first station ID in the 'if' statement
         //this is ok 99% of the time, but what about if there is statement like: 'if station X or station X'?
         //would then need to check that this station is a member of this pass too, and if not use the next
        if(*it == "station")
        {
            it++;
            if(it != condition_values.end()){station_id = *it; return station_id;}
        }
    }
    return station_id;
}


}//end namespace
