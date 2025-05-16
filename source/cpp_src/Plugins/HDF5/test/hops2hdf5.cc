#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"
#include "MHO_HDF5TypeCode.hh"

#include <stdio.h>
#include <utility>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "hdf5.h"
#include "MHO_HDF5ContainerFileInterface.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"


int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    //meta data detail level
    int detail = eJSONAxesWithLabelsLevel;
    std::string uuid_string = "";
    std::string shortname = "";
    int message_level = 0;

    CLI::App app{"hops2hdf5"};
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-u,--uuid", uuid_string, "specify and extract a single object by UUID");
    app.add_option("-s,--shortname", shortname,
                   "specify and extract a single object by shortname (returns first matching object) ")
        ->excludes("--uuid");
    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be converted")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output file, if not given the result will be stored in <input-file>.hdf5");

    CLI11_PARSE(app, argc, argv);
    

    // app.add_option("-d,--detail", detail,
    //                 "level of detail to be used when generating the output, range: 0 (low) to 3 (high), default (3)");
    // //clamp detail level
    // if(detail > eJSONAxesWithLabelsLevel){detail = eJSONAxesWithLabelsLevel;}
    // if(detail < eJSONBasicLevel){detail = eJSONBasicLevel;}

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(input_file == "")
    {
        msg_fatal("main", "input_file not set" << eom);
        return 1;
    }

    //set default output name, if not passed
    if(output_file == "")
    {
        output_file = input_file + ".hdf5";
    }

    //read in all the objects in a file
    MHO_ContainerStore conStore;
    MHO_HDF5ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore);
    
    int status = conInter.ConvertStoreToHDF5(conStore, output_file);

    msg_info("main", "done" << eom);

    return 0;
}
