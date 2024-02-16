#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;


#include "CLI11.hpp"

//
// /** This example demonstrates the use of `prefix_command` on a subcommand
// to capture all subsequent arguments along with an alias to make it appear as a regular options.
//
// All the values after the "sub" or "--sub" are available in the remaining() method.
// */
// int main(int argc, const char *argv[]) {
//
//     int value{0};
//     CLI::App app{"Test App"};
//     app.add_option("-v", value, "value");
//
//     auto *subcom = app.add_subcommand("sub", "")->prefix_command();
//     subcom->alias("--sub");
//     CLI11_PARSE(app, argc, argv);
//
//     std::cout << "value=" << value << '\n';
//     std::cout << "after Args:";
//     for(const auto &aarg : subcom->remaining()) {
//         std::cout << aarg << " ";
//     }
//     std::cout << '\n';
// }

int main(int argc, char** argv)
{
    std::string filename = "";
    std::string output_file = "";
    int detail = eJSONAll;

    CLI::App app{"hops2json"};


    app.add_option("-f", filename, "filename");
    app.add_option("-o", filename, "output_file");
    app.add_option("-d", detail, "detail");

    // auto *subcom = app.add_subcommand("sub", "")->prefix_command();
    // subcom->alias("--sub");
    CLI11_PARSE(app, argc, argv);

    // std::cout << "value=" << value << '\n';
    // std::cout << "after Args:";
    // for(const auto &aarg : subcom->remaining()) {
    // std::cout << aarg << " ";
    // }
    // std::cout << '\n';



    std::string usage = "hops2json -f <file> -d <detail level: 0 (low) to 4 (high)> -o <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    // std::string filename = "";
    // std::string output_file = "";
    // int detail = eJSONAll;
    //
    // static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
    //                                       {"file", required_argument, 0, 'f'},
    //                                       {"detail", required_argument, 0, 'd'},
    //                                       {"output", required_argument, 0, 'o'}
    // };
    //
    // static const char* optString = "hf:d:o:";
    //
    // while(true)
    // {
    //     char optId = getopt_long(argc, argv, optString, longOptions, NULL);
    //     if (optId == -1)
    //         break;
    //     switch(optId)
    //     {
    //         case ('h'):  // help
    //             std::cout << usage << std::endl;
    //             return 0;
    //         case ('f'):
    //             filename = std::string(optarg);
    //             break;
    //         case ('d'):
    //             detail = std::atoi(optarg);
    //             break;
    //         case ('o'):
    //             output_file = std::string(optarg);
    //             break;
    //         default:
    //             std::cout << usage << std::endl;
    //             return 1;
    //     }
    // }

    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(filename);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    //all file objects are now in memory
    std::cout<<"Converting "<<conStore.GetNObjects()<<" objects."<<std::endl;

    //convert the entire store to json
    mho_json root;
    conInter.ConvertStoreToJSON(conStore,root,detail);

    //open and dump to file
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);

    if(outFile.is_open())
    {
        outFile << root;
    }
    else
    {
        msg_error("main", "could not open file: " << output_file << eom);
    }
    outFile.close();

    std::cout<<"Done."<<std::endl;

    return 0;
}
