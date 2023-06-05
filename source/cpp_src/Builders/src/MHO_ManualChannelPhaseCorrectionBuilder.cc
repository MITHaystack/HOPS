#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_ManualChannelPhaseCorrectionBuilder::Build()
{

    //assume attributes are ok for now - TODO add checks!
    std::string op_name = fAttributes["name"].get<std::string>();
    std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
    std::vector<double> pc_phases = fAttributes["pc_phases"].get< std::vector<double> >();

    std::vector< std::string > chan_names;
    if(channel_name_str.find(",") != std::string::npos)
    {
        chan_names= SplitString(channel_name_str, std::string(",")); //split on commas
    }
    else 
    {
        chan_names= SplitString(channel_name_str); //split every char
    }


    std::string pol = ParsePolFromName(op_name);
    std::string mk4id = ExtractStationMk4ID();
    op_name = "pc_phases";
    
    std::cout<<"pol = "<<pol<<" station mk4id = "<<mk4id<<std::endl;

    if( pc_phases.size() == chan_names.size() )
    {
        MHO_ManualChannelPhaseCorrection* op = new MHO_ManualChannelPhaseCorrection();

        auto chan2pcp = zip_into_map(chan_names, pc_phases); //name -> freq
        op->SetChannelToPCPhaseMap(chan2pcp);
        op->SetPolarization(pol);
        op->SetStationMk4ID(mk4id);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(op,op_name,replace_duplicates);

        return true;

    }
    else
    {
        msg_error("builders", "cannot set pc_phases with unequal number of channels/elements " <<
                  "(channels, pc_phases) = (" << chan_names.size() << ", " << pc_phases.size() << ")"
                  << eom );
        return false;
    }
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
