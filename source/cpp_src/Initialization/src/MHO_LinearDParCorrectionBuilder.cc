#include "MHO_LinearDParCorrectionBuilder.hh"
#include "MHO_LinearDParCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_LinearDParCorrectionBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a linear pol dpar correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        double priority = fFormat["priority"].get< double >();
        std::string op_category = "calibration";

        std::string polprod;
        std::vector< std::string > pp_set;

        bool pps_ok = this->fParameterStore->IsPresent("/config/polprod_set");

        if(!pps_ok)
        {
            msg_error("initialization",
                      "polarization product information missing for linear dpar correction operation." << eom);
            return false;
        }

        pp_set = this->fParameterStore->GetAs< std::vector< std::string > >("/config/polprod_set");

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_LinearDParCorrection without visibility data." << eom);
            return false;
        }

        //get the parallactic angle values for each station
        bool ref_pa_ok = this->fParameterStore->IsPresent("/ref_station/parallactic_angle");
        bool rem_pa_ok = this->fParameterStore->IsPresent("/rem_station/parallactic_angle");
        if(!ref_pa_ok || !rem_pa_ok)
        {
            msg_error("initialization", "parallactic_angle information missing for linear dpar correction operation." << eom);
            return false;
        }
        double ref_pa = this->fParameterStore->GetAs< double >("/ref_station/parallactic_angle");
        double rem_pa = this->fParameterStore->GetAs< double >("/rem_station/parallactic_angle");

        MHO_LinearDParCorrection* op = new MHO_LinearDParCorrection();
        //set the arguments
        op->SetArgs(vis_data);
        op->SetPolProductSet(pp_set);
        op->SetReferenceParallacticAngle(ref_pa);
        op->SetRemoteParallacticAngle(rem_pa);

        op->SetName(op_name);
        op->SetPriority(priority);

        bool replace_duplicates = true;
        this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
