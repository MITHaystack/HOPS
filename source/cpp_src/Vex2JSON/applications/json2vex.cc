#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"
#include "MHO_VexGenerator.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "json2vex -i <input_file> -o <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    int message_level = 0;
    std::string input_file;
    std::string output_file;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_file", required_argument, 0, 'i'},
                                          {"output_file", required_argument, 0, 'o'},
                                          {"message_level", required_argument, 0, 'm'},
                                      };

    static const char* optString = "hi:o:m:";

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
            case ('m'):
                message_level = std::atoi(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }
    
    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    std::ifstream ifs;
    ifs.open( input_file.c_str(), std::ifstream::in );

    mho_json root;
    if(ifs.is_open())
    {
        root = mho_json::parse(ifs);
    }
    ifs.close();

    MHO_VexGenerator gen;
    gen.SetFilename(output_file);
    gen.GenerateVex(root);

    return 0;
}
