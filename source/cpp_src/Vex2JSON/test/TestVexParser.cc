#include <fstream>
#include <iostream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string vexfile(argv[1]);

    MHO_VexParser vparser;
    //vparser.SetVexVersion("2.0");
    vparser.SetVexFile(vexfile);
    mho_json vex = vparser.ParseVex();

    //open and dump to file
    std::string output_file("test_vex.json");
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << vex;
    outFile.close();

    return 0;
}
