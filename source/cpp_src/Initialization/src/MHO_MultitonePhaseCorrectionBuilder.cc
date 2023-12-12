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
        std::string mk4id = ExtractStationMk4ID(op_name);

        //check pc_mode values to see if this operator should be built at all (defaults to true)
        //first we check if there is a 'pc_mode' defined under '/control/station/pc_mode'
        std::string pc_mode = "multitone";
        if(this->fParameterStore->IsPresent("/control/station/pc_mode"))
        {
            pc_mode = this->fParameterStore->GetAs<std::string>("/control/station/pc_mode");
        }
        //however, any station specific value under '/control/station/<mk4id>/pc_mode' will 
        //override the generic /control/station/pc_mode
        std::string station_pcmode_path = std::string("/control/station/") + mk4id + "/pc_mode";
        if(this->fParameterStore->IsPresent(station_pcmode_path) )
        {
            pc_mode = this->fParameterStore->GetAs<std::string>(station_pcmode_path);
        }

        if(pc_mode == "multitone")
        {
            //grab the correct pcal data
            multitone_pcal_type* pcal_data = nullptr;
            if(op_name == "ref_multitone_pcal")
            {
                pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("ref_pcal"));
            }
            
            if(op_name == "rem_multitone_pcal")
            {
                pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("rem_pcal"));
            }

            //this is not necessarily an error...many stations do not have pcal
            if(pcal_data == nullptr )
            {
                msg_debug("initialization", "cannot construct MHO_MultitonePhaseCorrection without pcal data for station: "<<mk4id<< "." << eom);
                return false;
            }
            
            //retrieve the visibility data from the container store
            visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
            if( vis_data == nullptr )
            {
                msg_error("initialization", "cannot construct MHO_MultitonePhaseCorrection without visibility data." << eom);
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
        
        //multitone pcal not triggered for this station
        msg_debug("initialization", "MHO_MultitonePhaseCorrection will not be applied to station: "<<mk4id<<"." << eom);
        return false;
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
