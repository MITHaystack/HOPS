#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_ControlFileParser.hh"

#include "MHO_ControlConditionEvaluator.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    if(argc < 2){std::cout<<"must pass file name of control file"<<std::endl; return 1;}

    std::string controlfile(argv[1]);

    MHO_ControlFileParser cparser;
    cparser.SetControlFile(controlfile);
    auto control_statements = cparser.ParseControl();

    //    std::cout << control_statements.dump(2) << std::endl;

    MHO_ControlConditionEvaluator eval;
    for(auto it = control_statements["conditions"].begin(); it != control_statements["conditions"].end(); it++)
    {
        //std::cout << it->dump(2) << std::endl;
        if( it->find("statement_type") != it->end() )
        {
            if( !( (*it)["statement_type"].is_null() ) )
            {
                if( (*it)["statement_type"].get<std::string>() == "conditional" )
                {
                    bool b = eval.Evaluate( *it );
                }
            }
        }
    }

    //open and dump to file
    std::string output_file("./control.json");
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << control_statements;
    outFile.close();

    return 0;
}
