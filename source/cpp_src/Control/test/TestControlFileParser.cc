#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_ControlFileParser.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string controlfile(argv[1]);

    MHO_ControlFileParser cparser;
    cparser.SetControlFile(controlfile);
    auto control_statements = cparser.ParseControl();
    
    //open and dump to file 
    std::string output_file("./control.json");
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << control_statements;
    outFile.close();

    return 0;
}
