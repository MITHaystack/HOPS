#include <fstream>
#include <iostream>
#include <string>

#include "MHO_ControlFileParser.hh"
#include "MHO_Message.hh"

#include "MHO_ControlConditionEvaluator.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    if(argc < 2)
    {
        std::cout << "must pass file name of control file" << std::endl;
        return 1;
    }

    std::string controlfile(argv[1]);

    MHO_ControlFileParser cparser;
    cparser.SetControlFile(controlfile);
    auto control_statements = cparser.ParseControl();

    //    std::cout << control_statements.dump(2) << std::endl;

    MHO_ControlConditionEvaluator eval;
    eval.SetPassInformation(std::string("GE"), std::string("?"), std::string("X"), std::string("288-210210"));
    mho_json selected_ctrl_statements = eval.GetApplicableStatements(control_statements);

    //open and dump to file
    std::string output_file("./all_control.json");
    std::string output2_file("./selected_control.json");

    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << control_statements;
    outFile.close();

    std::ofstream outFile2(output2_file.c_str(), std::ofstream::out);
    outFile2 << selected_ctrl_statements;
    outFile2.close();

    return 0;
}
