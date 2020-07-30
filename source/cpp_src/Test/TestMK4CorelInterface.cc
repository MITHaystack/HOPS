#include <iostream>
#include <string>
#include <getopt.h>

#include "HkMK4CorelInterface.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestMK4CorelInterface -f <fringe_filename>";

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

    HkMK4CorelInterface mk4inter;
    mk4inter.ReadCorelFile(filename);

    Type100MetaData meta;
    std::vector< Type101Map > type101vector;
    std::vector< Type120Map > type120vector;
    mk4inter.ExportCorelFile(meta, type101vector, type120vector);


    std::cout<<"size of vectors (101,120) = "<<type101vector.size()<<", "<<type120vector.size()<<std::endl;


    return 0;
}
