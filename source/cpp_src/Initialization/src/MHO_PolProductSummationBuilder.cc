#include "MHO_PolProductSummationBuilder.hh"
#include "MHO_PolProductSummation.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_PolProductSummationBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building a manual per-channel phase correction operator."<< eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get<std::string>();
        double priority = fFormat["priority"].get<double>();
        std::string op_category = "calibration";

        std::string polprod;
        std::vector< std::string > pp_set;

        bool pp_ok = this->fParameterStore->IsPresent("/config/polprod"); 
        bool pps_ok = this->fParameterStore->IsPresent("/config/polprod_set");
        if(!pp_ok || !pps_ok)
        {
            msg_error("initialization", "polarization product information missing for summation operation." << eom );
            return false;
        }

        polprod = this->fParameterStore->GetAs<std::string>("/config/polprod");
        pp_set = this->fParameterStore->GetAs< std::vector< std::string > >("/config/polprod_set");

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        if( vis_data == nullptr )
        {
            msg_error("initialization", "cannot construct MHO_PolProductSummation without visibility data." << eom);
            return false;
        }

        MHO_PolProductSummation* op = new MHO_PolProductSummation();


        //set the arguments
        op->SetArgs(vis_data);
        op->SetPolProductSumLabel(polprod);
        op->SetPolProductSet(pp_set);

        // op->SetPCPhaseOffset(pc_phase_offset);
        // op->SetPolarization(pol);
        // op->SetStationMk4ID(mk4id);
        op->SetName(op_name);
        op->SetPriority(priority);

        //msg_debug("initialization", "creating operator: "<<op_name<<" for station: "<<mk4id<<" pol: "<<pol<<"."<<eom);

        bool replace_duplicates = true;
        this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
        return true;

    }
    return false;
}

// std::string
// MHO_PolProductSummationBuilder::ParsePolFromName(const std::string& name)
// {
//     if(name == "pc_phase_offset_x"){return std::string("X");}
//     if(name == "pc_phase_offset_y"){return std::string("Y");}
//     if(name == "pc_phase_offset_r"){return std::string("R");}
//     if(name == "pc_phase_offset_l"){return std::string("L");}
//     return std::string("?");
// }


// std::string
// MHO_PolProductSummationBuilder::ExtractStationMk4ID()
// {
//     std::string station_id = "?";
//     std::vector< std::string > condition_values = fConditions["value"].get< std::vector< std::string > >();
// 
//     for(auto it = condition_values.begin(); it != condition_values.end(); it++)
//     {
//          //grab the first station ID in the 'if' statement
//          //this is ok 99% of the time, but what about if there is statement like: 'if station X or station X'?
//          //would then need to check that this station is a member of this pass too, and if not use the next
//         if(*it == "station")
//         {
//             it++;
//             if(it != condition_values.end()){station_id = *it; return station_id;}
//         }
//     }
//     return station_id;
// }


}//end namespace
