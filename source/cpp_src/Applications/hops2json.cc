#include "MHO_Message.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerFileInterface.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"


int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    int detail = eJSONAll;
    int nspaces = 0;
    int message_level = 0;

    CLI::App app{"hops2json"};

    app.add_option("input,-i,--input-file", input_file, "The name of the input (hops) file to be converted.")->required();
    app.add_option("output,-o,--output-file", output_file, "The name of the output (json) file, if not given the result will be stored in <input-file>.json.");
    app.add_option("-d,--detail", detail, "The level of detail to be used when generating the output, range: 0 (low) to 4 (high), default (4).")->expected(0,4);
    app.add_option("-m,--message-level", message_level, "The message level to be used, range: -2 (debug) to 5 (silent).");
    app.add_option("-p,--pretty-print", nspaces, "Generate the json with indentations (soft tabs) consisting of the number of spaces specified, default (disabled).");

    CLI11_PARSE(app, argc, argv);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(output_file == "")
    {
        output_file = input_file + ".json";
    }

    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    //all file objects are now in memory
    msg_status("main", "converting "<<conStore.GetNObjects()<<" objects" << eom);

    //convert the entire store to json
    mho_json root;
    conInter.ConvertStoreToJSON(conStore,root,detail);

    //open and dump to file
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);

    if(outFile.is_open())
    {
        if(nspaces == 0){outFile << root;}
        else{ outFile << root.dump(nspaces); }
    }
    else
    {
        msg_error("main", "could not open file: " << output_file << eom);
    }

    outFile.close();

    msg_status("main", "done" << eom);

    return 0;
}
