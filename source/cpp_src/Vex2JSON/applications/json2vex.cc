#include <fstream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_VexGenerator.hh"
#include "MHO_VexParser.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    int message_level = 0;

    CLI::App app{"json2vex"};

    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("input,-i,--input-file", input_file, "name of the input (json) file to be converted")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output (vex) file, if not given the result will be stored in <input-file>.vex");

    CLI11_PARSE(app, argc, argv);

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
        output_file = input_file + ".vex";
    }

    //read and parse the input json
    msg_status("main", "reading json file: " << input_file << eom);
    std::ifstream ifs;
    ifs.open(input_file.c_str(), std::ifstream::in);

    mho_json root;
    if(ifs.is_open())
    {
        root = mho_json::parse(ifs);
    }
    else
    {
        msg_fatal("main", "could not open file: " << input_file << eom);
        return 1;
    }
    ifs.close();

    //generate and write the vex output
    msg_status("main", "generating vex file: " << output_file << eom);
    MHO_VexGenerator gen;
    gen.SetFilename(output_file);
    gen.GenerateVex(root);

    msg_info("main", "done" << eom);

    return 0;
}
