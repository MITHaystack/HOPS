#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"


using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string vexfile(argv[1]);

    MHO_VexParser vparser;
    vparser.SetVexVersion("2.0");
    vparser.SetVexFile(vexfile);
    vparser.ParseVex();

    return 0;
}
