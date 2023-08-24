#include "ffit.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"

void build_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    build_manager.BuildOperatorCategory(cat);
    std::cout<<"toolbox has: "<<opToolbox->GetNOperators()<<" operators."<<std::endl;
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        std::cout<<"init and exec of: "<<(*opIt)->GetName()<<std::endl;
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }    
}
