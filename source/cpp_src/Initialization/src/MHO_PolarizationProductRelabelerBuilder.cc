#include "MHO_PolarizationProductRelabelerBuilder.hh"
#include "MHO_PolarizationProductRelabeler.hh"
#include "MHO_Meta.hh"

namespace hops
{

bool MHO_PolarizationProductRelabelerBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a polarization_product_relabel operator." << eom);

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "calibration";
        double priority = fFormat["priority"].get<double>();

        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        if(vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_PolarizationProductRelabeler without visibility data." << eom);
            return false;
        }

        auto pol_relabeler = new MHO_PolarizationProductRelabeler<visibility_type>();

        std::string station_id = fAttributes["value"]["station"].get<std::string>();
        pol_relabeler->SetStationIdentifier(station_id);

        std::string pol1 = fAttributes["value"]["pol1"].get<std::string>();
        std::string pol2 = fAttributes["value"]["pol2"].get<std::string>();
        pol_relabeler->SetPolarizationSwapPair(pol1, pol2);
        
        pol_relabeler->SetArgs(vis_data);
        pol_relabeler->SetName(op_name);
        pol_relabeler->SetPriority(priority);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(pol_relabeler, pol_relabeler->GetName(), op_category, replace_duplicates);

        return true;
    }
    return false;
}

}