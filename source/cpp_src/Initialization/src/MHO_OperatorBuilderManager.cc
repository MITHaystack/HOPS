#include "MHO_OperatorBuilderManager.hh"


//builders
#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"

namespace hops
{

void
MHO_OperatorBuilderManager::CreateBuilders()
{
    AddBuilderType<MHO_ChannelLabellerBuilder>("chan_ids");
    AddBuilderType<MHO_ManualChannelPhaseCorrectionBuilder>("pc_phases_x"); //TODO FIXME -- need a different label
}

void
MHO_OperatorBuilderManager::BuildAll()
{
    //loop over control statements, find the associated builder in the builder map
    //and tell it to make an operator responsible for its action
    for(auto ctrl_iter = fControl.begin(); ctrl_iter != fControl.end(); ctrl_iter++)
    {
        auto ctrl_item = *(ctrl_iter);
        // std::cout<<ctrl_item.dump(2)<<std::endl;
        // std::cout<<"-----------------"<<std::endl;
        auto statements = (*ctrl_iter)["statements"];
        for(auto stmt_iter = statements.begin(); stmt_iter != statements.end(); stmt_iter++)
        {
            std::string name = (*stmt_iter)["name"];
            std::cout<<"looking for an operator builder for: "<<name<<std::endl;
            auto builder_it = fBuilderMap.find(name);
            if(builder_it != fBuilderMap.end())
            {
                std::cout<<"found an operator builder for: "<<name<<std::endl;
                builder_it->second->SetConditions(*ctrl_iter); //is this the right way to do this?
                builder_it->second->SetAttributes(*stmt_iter);
                builder_it->second->Build();
            }
        }
    }
}

}
