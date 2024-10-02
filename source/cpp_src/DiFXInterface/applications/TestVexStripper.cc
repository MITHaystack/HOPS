#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"

#include "MHO_DiFXInterface.hh"
#include "MHO_DiFXVexStripper.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestVexStripper <input_file> <output_file>";

    if(argc != 3)
    {
        std::cout << "Usage: " << usage << std::endl;
        return 1;
    }

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    MHO_DiFXVexStripper stripper;
    stripper.SetSessionVexFile(input_file);
    stripper.SetOutputFileName(output_file);
    stripper.ExtractScan();

    return 0;
}
