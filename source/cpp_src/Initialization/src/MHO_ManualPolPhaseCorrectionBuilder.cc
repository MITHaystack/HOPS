#include "MHO_ManualPolPhaseCorrectionBuilder.hh"
#include "MHO_ManualPolPhaseCorrection.hh"

#include <memory>

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_ManualPolPhaseCorrectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a manual per-pol phase correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "calibration";
        double pc_phase_offset = fAttributes["value"].get< double >();
        double priority = fFormat["priority"].get< double >();

        std::string pol = ParsePolFromName(op_name);

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_ManualPolPhaseCorrection without visibility data." << eom);
            return false;
        }

        std::unique_ptr< MHO_ManualPolPhaseCorrection > op(new MHO_ManualPolPhaseCorrection());

        //set the arguments
        op->SetArgs(vis_data);
        op->SetPCPhaseOffset(pc_phase_offset);
        op->SetPolarization(pol);
        op->SetStationIdentifiers(ExtractAllStationIdentifiers());
        op->SetName(op_name);
        op->SetPriority(priority);

        msg_debug("initialization", "creating operator: " << op_name << " pol: " << pol << "." << eom);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(std::move(op), op_name, op_category, replace_duplicates);
        return true;
    }
    return false;
}

std::string MHO_ManualPolPhaseCorrectionBuilder::ParsePolFromName(const std::string& name)
{
    if(name == "pc_phase_offset_x")
    {
        return std::string("X");
    }
    if(name == "pc_phase_offset_y")
    {
        return std::string("Y");
    }
    if(name == "pc_phase_offset_r")
    {
        return std::string("R");
    }
    if(name == "pc_phase_offset_l")
    {
        return std::string("L");
    }
    return std::string("?");
}

} // namespace hops
