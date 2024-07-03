#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"

#include "MHO_AFileDefinitions.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_AFileDefinitions adef;

    std::string file_type = "frng";
    mho_json format = adef.GetAFileFormat(file_type);

    return 0;
}
