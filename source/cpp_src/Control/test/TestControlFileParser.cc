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
    eval.SetPassInformation( std::string("GE"), std::string("?"), std::string("?"), std::string("288-210210"));


    mho_json selected_ctrl_statements;
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
                    if(b)
                    {
                        for(auto st = (*it)["statements"].begin(); st != (*it)["statements"].end(); st++)
                        {
                            selected_ctrl_statements.push_back(*st);
                        }
                    }
                    if(b){std::cout<<"statement is true"<<std::endl;}
                    else{std::cout<<"statement is false"<<std::endl;}
                }
            }
        }
    }

    std::cout<<"selected control statements:" <<std::endl;
    std::cout<< selected_ctrl_statements.dump(2) <<std::endl;



    //open and dump to file
    std::string output_file("./control.json");
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << control_statements;
    outFile.close();

    return 0;
}
