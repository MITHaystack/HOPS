#include "MHO_OperatorBuilderManager.hh"


//builders
#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_DataSelectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"

namespace hops
{

void
MHO_OperatorBuilderManager::CreateBuilders()
{

    //we have a very limited number of operators enabled currently
    AddBuilderType<MHO_ChannelLabellerBuilder>("chan_ids", fFormat["chan_ids"]);
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_x", fFormat["pc_phases_x"]); //TODO FIXME -- need a different label?
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_y", fFormat["pc_phases_y"]);
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_r", fFormat["pc_phases_r"]);
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_l", fFormat["pc_phases_l"]);
    
    //this one is special since it is not an operator specified via control file
    mho_json special;
    special["operator_category"] = "selection";
    special["priority"] = 1.1;
    AddBuilderType<MHO_DataSelectionBuilder>("coarse_selection", special); 
}

void 
MHO_OperatorBuilderManager::BuildOperatorCategory(const std::string& cat)
{
    //check that the category is supported (listed below)
    bool ok = false;
    if(cat == "default"){ok = true;}
    if(cat == "labelling"){ok = true;}
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
                    //no control attributes need (default builder)
                    it1->second->Build();
                    it1++;
                }
            } 
        }
        else 
        {
            for(auto ctrl_iter = fControl->begin(); ctrl_iter != fControl->end(); ctrl_iter++)
            {
                //auto ctrl_item = *(ctrl_iter);
                // std::cout<<ctrl_item.dump(2)<<std::endl;
                // std::cout<<"-----------------"<<std::endl;
                auto statements = &( (*ctrl_iter)["statements"] );
                for(auto stmt_iter = statements->begin(); stmt_iter != statements->end(); )
                {
                    std::string name = (*stmt_iter)["name"];
                    bool build_op = false;
                    if( fFormat.contains(name) && fFormat[name].contains("operator_category") )
                    {
                        if(cat == fFormat[name]["operator_category"].get<std::string>() ){build_op = true;}
                    }
                    
                    if(build_op)
                    {
                        std::cout<<"looking for an operator builder for: "<<name<<" category: "<<cat<<std::endl;
                        auto builder_it = fNameToBuilderMap.find(name);
                        if(builder_it != fNameToBuilderMap.end())
                        {
                            std::cout<<"found an operator builder for: "<<name<<std::endl;
                            builder_it->second->SetConditions(*ctrl_iter);
                            builder_it->second->SetAttributes(*stmt_iter);
                            builder_it->second->Build();
                        }
                        stmt_iter = statements->erase(stmt_iter); std::cout<<"erased consumed op statement: "<<name<<std::endl;
                    }
                    else 
                    {
                        stmt_iter++;
                        msg_debug("initialization", "operator: "<< name <<" not supported or part of "<<cat<<" category."<<eom);
                    }
                }
            }
        }
    }
    else 
    {
        msg_warn("initialization", "operator category: "<< cat<< " is not supported." << eom);
    }
    
}




// void
// MHO_OperatorBuilderManager::BuildControlStatementOperators()
// {
//     std::cout<<"bulding control ops"<<std::endl;
//     //loop over control statements, find the associated builder in the builder map
//     //and tell it to make an operator responsible for its action
//     for(auto ctrl_iter = fControl->begin(); ctrl_iter != fControl->end(); ctrl_iter++)
//     {
//         auto ctrl_item = *(ctrl_iter);
//         // std::cout<<ctrl_item.dump(2)<<std::endl;
//         // std::cout<<"-----------------"<<std::endl;
//         auto statements = (*ctrl_iter)["statements"];
//         for(auto stmt_iter = statements.begin(); stmt_iter != statements.end(); stmt_iter++)
//         {
//             std::string name = (*stmt_iter)["name"];
//             std::cout<<"looking for an operator builder for: "<<name<<std::endl;
//             auto builder_it = fNameToBuilderMap.find(name);
//             if(builder_it != fNameToBuilderMap.end())
//             {
//                 std::cout<<"found an operator builder for: "<<name<<std::endl;
//                 builder_it->second->SetConditions(*ctrl_iter); //is this the right way to do this?
//                 builder_it->second->SetAttributes(*stmt_iter);
//                 builder_it->second->Build();
//             }
//         }
//     }
// }

}
