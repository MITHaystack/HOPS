#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_VexGenerator.hh"


using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string jsonfile(argv[1]);

    std::ifstream ifs;
    ifs.open( jsonfile.c_str(), std::ifstream::in );

    mho_json root;
    if(ifs.is_open())
    {
        root = mho_json::parse(ifs);
    }
    ifs.close();

    MHO_VexGenerator gen;
    std::string output_file("./test.vex");
    gen.SetFilename(output_file);

    gen.GenerateVex(root);


    return 0;
}
