#include "MHO_MultitonePhaseCorrectionBuilder.hh"
#include "MHO_MultitonePhaseCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_MultitonePhaseCorrectionBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building a multitone phase correction operator."<< eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "calibration";
        // std::string channel_name_str = fAttributes["value"]["channel_names"].get<std::string>();
        // std::vector<double> pc_phases = fAttributes["value"]["pc_phases"].get< std::vector<double> >();

        std::string mk4id = ExtractStationMk4ID(op_name);

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));

        //grab the correct pcal data
        multitone_pcal_type* pcal_data = nullptr;
        if(op_name == "ref_multitone_pcal")
        {
            multitone_pcal_type* pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("ref_pcal"));
        }
        if(op_name == "rem_multitone_pcal")
        {
            multitone_pcal_type* pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("rem_pcal"));
        }

        if( vis_data == nullptr )
        {
            msg_error("initialization", "cannot construct MHO_MultitonePhaseCorrection without visibility data." << eom);
            return false;
        }

        if( pcal_data == nullptr )
        {
            msg_error("initialization", "cannot construct MHO_MultitonePhaseCorrection without pcal data." << eom);
            return false;
        }

        MHO_MultitonePhaseCorrection* op = new MHO_MultitonePhaseCorrection();

        //set the arguments

        std::cout<<" ********************** I'm making a p-cal operator!!!: " <<op_name<<" for station: "<<mk4id<<std::endl;
        op->SetArgs(vis_data);
        op->SetPCPeriod(1);
        op->SetStationMk4ID(mk4id);
        op->SetName(op_name);
        op->SetMultitonePCData(pcal_data);

        //msg_debug("initialization", "creating operator: "<<op_name<<" for station: "<<mk4id<<" pol: "<<pol<<"."<<eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
        return true;

    }
    return false;
}



std::string
MHO_MultitonePhaseCorrectionBuilder::ExtractStationMk4ID(std::string op_name)
{
    std::string station_id = "?";

    if(op_name == fRefOpName)
    {
        station_id = this->fParameterStore->GetAs<std::string>("/ref_station/mk4id");
    }

    if(op_name == fRemOpName)
    {
        station_id = this->fParameterStore->GetAs<std::string>("/rem_station/mk4id");
    }


    // std::vector< std::string > condition_values = fConditions["value"].get< std::vector< std::string > >();
    //
    // for(auto it = condition_values.begin(); it != condition_values.end(); it++)
    // {
    //      //grab the first station ID in the 'if' statement
    //      //this is ok 99% of the time, but what about if there is statement like: 'if station X or station X'?
    //      //would then need to check that this station is a member of this pass too, and if not use the next
    //     if(*it == "station")
    //     {
    //         it++;
    //         if(it != condition_values.end()){station_id = *it; return station_id;}
    //     }
    // }
     return station_id;
}


}//end namespace
