#include "MHO_PassbandBuilder.hh"
#include "MHO_Passband.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_PassbandBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        // msg_debug("initialization", "building a manual per-pol phase correction operator."<< eom);
        // //assume attributes are ok for now - TODO add checks!
        //
        // std::string op_name = fAttributes["name"].get<std::string>();
        // std::string op_category = "calibration";
        // double pc_phase_offset = fAttributes["value"].get<double>();
        // double priority = fFormat["priority"].get<double>();
        //
        // std::string pol = ParsePolFromName(op_name);
        // std::string mk4id = ExtractStationMk4ID();
        //
        // //retrieve the arguments to operate on from the container store
        // visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        // if( vis_data == nullptr )
        // {
        //     msg_error("initialization", "cannot construct MHO_ManualPolPhaseCorrection without visibility data." << eom);
        //     return false;
        // }
        //
        // MHO_ManualPolPhaseCorrection* op = new MHO_ManualPolPhaseCorrection();
        //
        // //set the arguments
        // op->SetArgs(vis_data);
        // op->SetPCPhaseOffset(pc_phase_offset);
        // op->SetPolarization(pol);
        // op->SetStationMk4ID(mk4id);
        // op->SetName(op_name);
        // op->SetPriority(priority);
        //
        // msg_debug("initialization", "creating operator: "<<op_name<<" for station: "<<mk4id<<" pol: "<<pol<<"."<<eom);
        //
        // bool replace_duplicates = false;
        // this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
        return true;

    }
    return false;
}





}//end namespace
