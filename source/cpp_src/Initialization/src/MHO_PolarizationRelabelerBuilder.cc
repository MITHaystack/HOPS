#include "MHO_PolarizationRelabelerBuilder.hh"
#include "MHO_Meta.hh"
#include "MHO_PolarizationProductRelabeler.hh"
#include "MHO_PolarizationRelabeler.hh"

#include <memory>

namespace hops
{

bool MHO_PolarizationRelabelerBuilder::Build()
{
    if(IsConfigurationOk())
    {
        msg_debug("initialization", "building a polarization_relabel operator." << eom);

        std::string op_name = fAttributes["name"].get< std::string >();
        std::string op_category = "labeling";
        double priority = fFormat["priority"].get< double >();

        auto station_ids = GetMatchingStationIdentifiers();
        std::string pol1 = fAttributes["value"]["pol1"].get< std::string >();
        std::string pol2 = fAttributes["value"]["pol2"].get< std::string >();

        visibility_type* vis_data = fContainerStore->GetObject< visibility_type >(std::string("vis"));
        weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));

        if(vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("initialization",
                      "cannot construct MHO_PolarizationProductRelabeler without visibility or weight data." << eom);
            return false;
        }

        //need an operator for both vis and weight data
        std::unique_ptr< MHO_PolarizationProductRelabeler< visibility_type > > vis_op(
            new MHO_PolarizationProductRelabeler< visibility_type >());
        std::unique_ptr< MHO_PolarizationProductRelabeler< weight_type > > wt_op(
            new MHO_PolarizationProductRelabeler< weight_type >());

        vis_op->SetStationIdentifiers(station_ids);
        vis_op->SetPolarizationSwapPair(pol1, pol2);
        vis_op->SetArgs(vis_data);
        vis_op->SetPriority(priority);

        wt_op->SetStationIdentifiers(station_ids);
        wt_op->SetPolarizationSwapPair(pol1, pol2);
        wt_op->SetArgs(wt_data);
        wt_op->SetPriority(priority);

        vis_op->SetName(op_name + ":vis");
        wt_op->SetName(op_name + ":weight");

        bool replace_duplicates = false;
        fOperatorToolbox->AddOperator(std::move(vis_op), op_name + ":vis", op_category, replace_duplicates);
        fOperatorToolbox->AddOperator(std::move(wt_op), op_name + ":weight", op_category, replace_duplicates);

        //if there is multitone pcal data available, then we also need to relabel the pols there
        multitone_pcal_type* ref_pcal_data = fContainerStore->GetObject< multitone_pcal_type >(std::string("ref_pcal"));
        if(ref_pcal_data != nullptr)
        {
            std::unique_ptr< MHO_PolarizationRelabeler< multitone_pcal_type > > ref_pcal_relabeler(
                new MHO_PolarizationRelabeler< multitone_pcal_type >());

            ref_pcal_relabeler->SetStationIdentifiers(station_ids);
            ref_pcal_relabeler->SetPolarizationSwapPair(pol1, pol2);
            ref_pcal_relabeler->SetArgs(ref_pcal_data);
            ref_pcal_relabeler->SetName(op_name);
            ref_pcal_relabeler->SetPriority(priority);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(std::move(ref_pcal_relabeler), op_name, op_category, replace_duplicates);
        }

        multitone_pcal_type* rem_pcal_data = fContainerStore->GetObject< multitone_pcal_type >(std::string("rem_pcal"));
        if(rem_pcal_data != nullptr)
        {
            std::unique_ptr< MHO_PolarizationRelabeler< multitone_pcal_type > > rem_pcal_relabeler(
                new MHO_PolarizationRelabeler< multitone_pcal_type >());

            rem_pcal_relabeler->SetStationIdentifiers(station_ids);
            rem_pcal_relabeler->SetPolarizationSwapPair(pol1, pol2);

            rem_pcal_relabeler->SetArgs(rem_pcal_data);
            rem_pcal_relabeler->SetName(op_name);
            rem_pcal_relabeler->SetPriority(priority);

            bool replace_duplicates = false;
            this->fOperatorToolbox->AddOperator(std::move(rem_pcal_relabeler), op_name, op_category, replace_duplicates);
        }

        return true;
    }
    return false;
}

} // namespace hops