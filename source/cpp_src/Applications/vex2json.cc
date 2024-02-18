#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "vex2json -i <input_file> -o <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_file;
    std::string output_file;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_file", required_argument, 0, 'i'},
                                          {"output_file", required_argument, 0, 'o'}};

    static const char* optString = "hi:o:";

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
            case ('i'):
                input_file = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //parse and convert
    MHO_VexParser vparser;
    vparser.SetVexFile(input_file);
    mho_json vex = vparser.ParseVex();

    //open and dump to file
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    outFile << vex;
    outFile.close();

    return 0;
}
