#include "MHO_PolarizationRelabelerBuilder.hh"
#include "MHO_PolarizationRelabeler.hh"
#include "MHO_Meta.hh"

namespace hops
{

bool MHO_PolarizationRelabelerBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a polarization_relabel operator." << eom);

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "labeling";
        double priority = fFormat["priority"].get<double>();

        multitone_pcal_type* pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("pcal"));
        if(pcal_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_PolarizationRelabeler without pcal data." << eom);
            return false;
        }

        auto pol_relabeler = new MHO_PolarizationRelabeler<multitone_pcal_type>();

        std::string station_id = fAttributes["value"]["station"].get<std::string>();
        pol_relabeler->SetStationIdentifier(station_id);

        std::string pol1 = fAttributes["value"]["pol1"].get<std::string>();
        std::string pol2 = fAttributes["value"]["pol2"].get<std::string>();
        pol_relabeler->SetPolarizationSwapPair(pol1, pol2);
        
        pol_relabeler->SetArgs(pcal_data);
        pol_relabeler->SetName(op_name);
        pol_relabeler->SetPriority(priority);

        bool replace_duplicates = false;
        this->fOperatorToolbox->AddOperator(pol_relabeler, pol_relabeler->GetName(), op_category, replace_duplicates);

        return true;
    }
    return false;
}

}