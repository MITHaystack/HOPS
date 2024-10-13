#include "MHO_PolProductSummationBuilder.hh"
#include "MHO_PolProductSummation.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_PolProductSummationBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a pol-product summation operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        double priority = fFormat["priority"].get< double >();
        std::string op_category = "calibration";

        std::string polprod;
        std::vector< std::string > pp_set;

        bool pp_ok = this->fParameterStore->IsPresent("/config/polprod");
        bool pps_ok = this->fParameterStore->IsPresent("/config/polprod_set");

        if(!pp_ok || !pps_ok)
        {
            msg_error("initialization", "polarization product information missing for pol-product summation operation." << eom);
            return false;
        }

        polprod = this->fParameterStore->GetAs< std::string >("/config/polprod");
        pp_set = this->fParameterStore->GetAs< std::vector< std::string > >("/config/polprod_set");

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_PolProductSummation without visibility data." << eom);
            return false;
        }

        weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
        if(wt_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_PolProductSummation without weight data." << eom);
            return false;
        }

        station_coord_type* ref_data = fContainerStore->GetObject< station_coord_type >(std::string("ref_sta"));
        if(ref_data == nullptr)
        {
            msg_error("initialization",
                      "cannot construct MHO_PolProductSummation without reference station coordinate data." << eom);
            return false;
        }

        station_coord_type* rem_data = fContainerStore->GetObject< station_coord_type >(std::string("rem_sta"));
        if(rem_data == nullptr)
        {
            msg_error("initialization",
                      "cannot construct MHO_PolProductSummation without remote station coordinate data." << eom);
            return false;
        }

        //get the parallactic angle values for each station
        bool ref_pa_ok = this->fParameterStore->IsPresent("/ref_station/parallactic_angle");
        bool rem_pa_ok = this->fParameterStore->IsPresent("/rem_station/parallactic_angle");
        if(!ref_pa_ok || !rem_pa_ok)
        {
            msg_error("initialization", "parallactic_angle information missing for pol-product summation operation." << eom);
            return false;
        }
        double ref_pa = this->fParameterStore->GetAs< double >("/ref_station/parallactic_angle");
        double rem_pa = this->fParameterStore->GetAs< double >("/rem_station/parallactic_angle");

        MHO_PolProductSummation* op = new MHO_PolProductSummation();
        //set the arguments
        op->SetArgs(vis_data);
        op->SetWeights(wt_data);
        op->SetReferenceStationCoordinateData(ref_data);
        op->SetRemoteStationCoordinateData(rem_data);
        op->SetPolProductSumLabel(polprod);
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
