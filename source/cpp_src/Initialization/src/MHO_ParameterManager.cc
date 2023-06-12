#include "MHO_ParameterManager.hh"

namespace hops
{

void
MHO_ParameterManager::ConfigureAll()
{
    //loop over control statements, find the associated builder in the builder map
    //and tell it to make an operator responsible for its action
    for(auto ctrl_iter = fControl.begin(); ctrl_iter != fControl.end(); ctrl_iter++)
    {
        auto ctrl_item = *(ctrl_iter);
        auto statements = (*ctrl_iter)["statements"];
        for(auto stmt_iter = statements.begin(); stmt_iter != statements.end(); stmt_iter++)
        {
            std::string name = (*stmt_iter)["name"];
            std::cout<<"param processing: "<<name<<std::endl;
            fDefaultParameterConfig.SetConditions(*ctrl_iter);
            fDefaultParameterConfig.SetAttributes(*stmt_iter);
            fDefaultParameterConfig.Configure();
        }
    }
}

}
