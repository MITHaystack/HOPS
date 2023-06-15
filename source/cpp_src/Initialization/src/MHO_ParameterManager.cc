#include "MHO_ParameterManager.hh"

namespace hops
{

void
MHO_ParameterManager::ConfigureAll()
{
    //loop over control statements, find the parameter statements and put them in 
    //parameter store, then remove them from further processing
    for(auto ctrl_iter = fControl->begin(); ctrl_iter != fControl->end(); ctrl_iter++)
    {
        auto ctrl_item = *(ctrl_iter);
        auto statements = &( (*ctrl_iter)["statements"] );
        std::vector< std::string > consumed_elements;
        for(auto stmt_iter = statements->begin(); stmt_iter != statements->end(); )
        {
            std::string name = (*stmt_iter)["name"];
            if(fFormat.contains(name) && fFormat[name].contains("statement_type") )
            {
                std::string stmt_type = fFormat[name]["statement_type"].get<std::string>();
                if(stmt_type == "parameter")
                {
                    msg_debug("initialization", "configuring parameter: "<<name<<"."<<eom);
                    consumed_elements.push_back(name);
                    fDefaultParameterConfig.SetConditions(*ctrl_iter);
                    fDefaultParameterConfig.SetAttributes(*stmt_iter);
                    fDefaultParameterConfig.Configure();
                    stmt_iter = statements->erase(stmt_iter);
                }
                else{stmt_iter++;}
                //TODO Expand the possible parameter types 
                //(e.g. we may want 'station_parameter' for some quantities, e.g. ionosphere)
            }
            else
            {
                stmt_iter++;
            }
        }
    }
}

}
