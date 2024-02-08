#include "MHO_OperatorBuilderManager.hh"


//builders
#include "MHO_ChannelLabelerBuilder.hh"
#include "MHO_DataSelectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_ManualChannelDelayCorrectionBuilder.hh"
#include "MHO_ManualPolPhaseCorrectionBuilder.hh"
#include "MHO_ManualPolDelayCorrectionBuilder.hh"
#include "MHO_MultitonePhaseCorrectionBuilder.hh"
#include "MHO_PolProductSummationBuilder.hh"
#include "MHO_SamplerLabelerBuilder.hh"
#include "MHO_LinearDParCorrectionBuilder.hh"

namespace hops
{

void
MHO_OperatorBuilderManager::CreateDefaultBuilders()
{


    //we have a very limited number of operators enabled currently
    AddBuilderType<MHO_ChannelLabelerBuilder>("chan_ids", "chan_ids");

    //manual per-channel pc phase corrections
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_x", "pc_phases_x");
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_y", "pc_phases_y");
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_r", "pc_phases_r");
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_l", "pc_phases_l");

    //manual per-channel pc delay corrections
    AddBuilderType<MHO_ManualChannelDelayCorrectionBuilder>("delay_offs_x", "delay_offs_x");
    AddBuilderType<MHO_ManualChannelDelayCorrectionBuilder>("delay_offs_y", "delay_offs_y");
    AddBuilderType<MHO_ManualChannelDelayCorrectionBuilder>("delay_offs_r", "delay_offs_r");
    AddBuilderType<MHO_ManualChannelDelayCorrectionBuilder>("delay_offs_l", "delay_offs_l");

    //manual per-pol pc phase corrections
    AddBuilderType<MHO_ManualPolPhaseCorrectionBuilder>("pc_phase_offset_x", "pc_phase_offset_x");
    AddBuilderType<MHO_ManualPolPhaseCorrectionBuilder>("pc_phase_offset_y", "pc_phase_offset_y");
    AddBuilderType<MHO_ManualPolPhaseCorrectionBuilder>("pc_phase_offset_r", "pc_phase_offset_r");
    AddBuilderType<MHO_ManualPolPhaseCorrectionBuilder>("pc_phase_offset_l", "pc_phase_offset_l");

    //manual per-pol pc delay corrections
    AddBuilderType<MHO_ManualPolDelayCorrectionBuilder>("pc_delay_x", "pc_delay_x");
    AddBuilderType<MHO_ManualPolDelayCorrectionBuilder>("pc_delay_y", "pc_delay_y");
    AddBuilderType<MHO_ManualPolDelayCorrectionBuilder>("pc_delay_r", "pc_delay_r");
    AddBuilderType<MHO_ManualPolDelayCorrectionBuilder>("pc_delay_l", "pc_delay_l");

    //add builders for operators which are not accessible via control file
    CreateNullFormatBuilders();
}



void
MHO_OperatorBuilderManager::BuildOperatorCategory(const std::string& cat)
{
    //check that the category is supported (listed below)
    bool ok = false;
    if(cat == "default"){ok = true;}
    if(cat == "labeling"){ok = true;}
    if(cat == "selection"){ok = true;}
    if(cat == "flagging"){ok = true;}
    if(cat == "calibration"){ok = true;}

    if(true)
    {
        msg_debug("initialization", "building operator category: "<<cat<<"."<<eom);
        //default category requires no control input
        if(cat == "default")
        {
            auto it1 = fCategoryToBuilderMap.lower_bound(cat);
            auto it2 = fCategoryToBuilderMap.upper_bound(cat);
            if(it1 != fCategoryToBuilderMap.end() )
            {
                while (it1 != it2)
                {
                    //no control attributes need (default builders)
                    it1->second->Build();
                    it1++;
                }
            }
        }
        else
        {
            for(auto ctrl_iter = fControl->begin(); ctrl_iter != fControl->end(); ctrl_iter++)
            {
                if( !( ctrl_iter->is_null() || ctrl_iter->is_null() ) )
                {
                    auto statements = &( (*ctrl_iter)["statements"] );
                    for(auto stmt_iter = statements->begin(); stmt_iter != statements->end(); )
                    {
                        std::string name = (*stmt_iter)["name"];
                        bool build_op = false;
                        if( fFormat.contains(name) && fFormat[name].contains("operator_category") )
                        {
                            if(cat == fFormat[name]["operator_category"].get<std::string>() )
                            {
                                build_op = true;
                            }
                        }

                        if(build_op)
                        {
                            auto builder_it = fNameToBuilderMap.find(name);
                            if(builder_it != fNameToBuilderMap.end())
                            {
                                msg_debug("initialization", "building operator with name: "<<name<<" in category: "<<cat<<"."<<eom);
                                builder_it->second->SetConditions(*ctrl_iter);
                                builder_it->second->SetAttributes(*stmt_iter);
                                bool build_status_ok = builder_it->second->Build();
                                if(!build_status_ok)
                                {
                                    msg_debug("initialization", "operator with name: "<<name<<" in category: "<<cat<<", was not built."<<eom);
                                }
                                stmt_iter = statements->erase(stmt_iter);
                            }
                            else //couldn't find a builder for this operator, skip
                            {
                                stmt_iter++;
                                msg_debug("initialization", "operator: "<< name <<" not yet supported."<<eom);
                            }
                        }
                        else
                        {
                            stmt_iter++; //statement not in this category, skip
                        }
                    }
                }
                else
                {
                    msg_error("initialization", "null control statement encountered " << eom );
                }
            }
        }
    }
    else
    {
        msg_warn("initialization", "operator category: "<< cat<< " is not supported." << eom);
    }

}


//creates builders for which there is no 'format' definition, since they are
//inaccesible from the control file
void MHO_OperatorBuilderManager::CreateNullFormatBuilders()
{
    //the below additions are some operators which have to be applied (usually by default)
    //but are not necessarily specified via control file (e.g. data selection and default channel labels)


    mho_json samplers;
    samplers["name"] = "sampler_labeler";
    samplers["operator_category"] = "labeling";
    samplers["priority"] = 0.9;
    AddBuilderTypeWithFormat<MHO_SamplerLabelerBuilder>("sampler_labeler", samplers);

    //this one is special since it is not an operator specified via control file
    mho_json special;
    special["name"] = "coarse_selection";
    special["operator_category"] = "selection";
    special["priority"] = 1.1;
    AddBuilderTypeWithFormat<MHO_DataSelectionBuilder>("coarse_selection", special);

    //this one is also special (default channel labeling behavior)
    mho_json special2;
    special2["name"] = "default_chan_ids";
    special2["operator_category"] = "default";
    special2["priority"] = 0.1;
    AddBuilderTypeWithFormat<MHO_ChannelLabelerBuilder>("default_chan_ids", special2);

    //we have to have reference and remote station multitone pcal operator builers defined here
    //since these are applied by default if pcal data is present, however they may be disabled
    //by control file statements like 'pc_mode manual'

    mho_json ref_mtpcal;
    ref_mtpcal["name"] = "ref_multitone_pcal";
    ref_mtpcal["operator_category"] = "calibration";
    ref_mtpcal["priority"] = 3.1;
    AddBuilderTypeWithFormat<MHO_MultitonePhaseCorrectionBuilder>("ref_multitone_pcal", ref_mtpcal);

    mho_json rem_mtpcal;
    rem_mtpcal["name"] = "rem_multitone_pcal";
    rem_mtpcal["operator_category"] = "calibration";
    rem_mtpcal["priority"] = 3.1;
    AddBuilderTypeWithFormat<MHO_MultitonePhaseCorrectionBuilder>("rem_multitone_pcal", rem_mtpcal);

    mho_json polprod_sum;
    polprod_sum["name"] = "polproduct_sum";
    polprod_sum["operator_category"] = "calibration";
    polprod_sum["priority"] = 3.99;
    AddBuilderTypeWithFormat<MHO_PolProductSummationBuilder>("polproduct_sum", polprod_sum);

    mho_json dpar_corr;
    dpar_corr["name"] = "dpar_corr";
    dpar_corr["operator_category"] = "calibration";
    dpar_corr["priority"] = 3.99;
    AddBuilderTypeWithFormat<MHO_LinearDParCorrectionBuilder>("dpar_corr", dpar_corr);

}



}//end namespace
