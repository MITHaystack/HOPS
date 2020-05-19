#include <iostream>
#include <string>
#include <getopt.h>

#include "HMK4Interface.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestMK4Interface -f <fringe_filename>";

    std::string filename;
    bool have_file = false;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"file", required_argument, 0, 'f'}};

    static const char* optString = "hf:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('f'):
                filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    std::cout<<"attempting to open: "<<filename<<std::endl;

    HMK4Interface mk4inter;
    mk4inter.OpenFringeFile(filename);

    return 0;
}
