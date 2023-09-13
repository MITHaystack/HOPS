#include "ffit.hh"

void init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    build_manager.BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("main", "initializing and executing operator: "<< (*opIt)->GetName() << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}
