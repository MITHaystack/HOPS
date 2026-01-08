#include "MHO_CircularFieldRotationBuilder.hh"
#include "MHO_CircularFieldRotationCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool MHO_CircularFieldRotationBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a circular pol field rotation correction operator." << eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get< std::string >();
        double priority = fFormat["priority"].get< double >();
        std::string op_category = "calibration";

        std::string polprod;
        std::string fourfit_reftime;
        std::vector< std::string > pp_set;

        bool pps_ok = this->fParameterStore->IsPresent("/config/polprod_set");
        if(!pps_ok)
        {
            msg_error("initialization",
                      "polarization product information missing for circular pol field rotation correction operation." << eom);
            return false;
        }
        pp_set = this->fParameterStore->GetAs< std::vector< std::string > >("/config/polprod_set");

        bool frt_ok = this->fParameterStore->IsPresent("/vex/scan/fourfit_reftime");
        if(!frt_ok)
        {
            msg_error("initialization",
                      "fourfit reference time information missing for circular pol field rotation correction operation."
                          << eom);
            return false;
        }
        fourfit_reftime = this->fParameterStore->GetAs< std::string >("/vex/scan/fourfit_reftime");

        //get the reference and remote station codes (2-char)
        std::string ref_id = this->fParameterStore->GetAs< std::string >("/ref_station/site_id");
        std::string rem_id = this->fParameterStore->GetAs< std::string >("/rem_station/site_id");
        std::string ref_mount_type = "";
        std::string rem_mount_type = "";

        //now determine the path to the antenna mount_type info, if it exists at all
        std::string generic_path = "/control/station/mount_type";
        std::string ref_path = "";
        std::string rem_path = "";
        if(this->fParameterStore->IsPresent(generic_path))
        {
            //generic info available (applies to all stations)
            ref_path = generic_path;
            rem_path = generic_path;
        }
        //specific ref-station info available
        if(this->fParameterStore->IsPresent(std::string("/control/station/") + ref_id + "/mount_type"))
        {
            ref_path = std::string("/control/station/") + ref_id + "/mount_type";
        }
        //specific rem-station info available
        if(this->fParameterStore->IsPresent(std::string("/control/station/") + rem_id + "/mount_type"))
        {
            rem_path = std::string("/control/station/") + rem_id + "/mount_type";
        }

        //bail out if no info available
        if(ref_path == "" && rem_path == "")
        {
            msg_debug("initialization",
                      "will not build circular field rotation operator, as there is no antenna mount type info present."
                          << eom);
            return false;
        }

        //next retrieve the necessary parameter values (mount_type) from the parameter store
        if(ref_path != "")
        {
            this->fParameterStore->Get(ref_path, ref_mount_type);
        }

        if(rem_path != "")
        {
            this->fParameterStore->Get(rem_path, rem_mount_type);
        }

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_CircularFieldRotationCorrection without visibility data." << eom);
            return false;
        }

        //get the station model data
        station_coord_type* ref_sta = fContainerStore->GetObject< station_coord_type >(std::string("ref_sta"));
        if(ref_sta == nullptr)
        {
            msg_error("initialization",
                      "cannot construct MHO_CircularFieldRotationCorrection without reference station data." << eom);
            return false;
        }
        station_coord_type* rem_sta = fContainerStore->GetObject< station_coord_type >(std::string("rem_sta"));
        if(rem_sta == nullptr)
        {
            msg_error("initialization",
                      "cannot construct MHO_CircularFieldRotationCorrection without remote station data." << eom);
            return false;
        }

        MHO_CircularFieldRotationCorrection* op = new MHO_CircularFieldRotationCorrection();

        //set the arguments
        op->SetArgs(vis_data);
        op->SetPolProductSet(pp_set);
        op->SetFourfitReferenceTimeVexString(fourfit_reftime);
        //set station coord data
        op->SetReferenceStationCoordinateData(ref_sta);
        op->SetRemoteStationCoordinateData(rem_sta);
        //set the station mount types
        op->SetReferenceMountType(ref_mount_type);
        op->SetRemoteMountType(rem_mount_type);
        op->SetName(op_name);
        op->SetPriority(priority);

        bool replace_duplicates = true;
        this->fOperatorToolbox->AddOperator(op, op->GetName(), op_category, replace_duplicates);
        return true;
    }
    return false;
}

} // namespace hops
