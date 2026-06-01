#include <fstream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    unsigned int nspaces = 0;
    int message_level = 0;

    CLI::App app{"vex2json"};

    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-p,--pretty-print", nspaces,
                   "generates the json ouput with indentations (soft tabs) consisting of the number of spaces specified, "
                   "default (disabled)");
    app.add_option("input,-i,--input-file", input_file, "name of the input (vex) file to be converted")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output (json) file, if not given the result will be stored in <input-file>.json");

    CLI11_PARSE(app, argc, argv);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(input_file == "")
    {
        msg_fatal("main", "input_file not set" << eom);
        return 1;
    }

    //make sure the input file exists/is readable before doing anything else
    {
        std::ifstream ifs(input_file.c_str(), std::ifstream::in);
        if(!ifs.is_open())
        {
            msg_fatal("main", "could not open input file: " << input_file << eom);
            return 1;
        }
    }

    //set default output name, if not passed
    if(output_file == "")
    {
        output_file = input_file + ".json";
    }

    //parse and convert
    msg_status("main", "parsing vex file: " << input_file << eom);
    MHO_VexParser vparser;
    vparser.SetVexFile(input_file);
    mho_json vex = vparser.ParseVex();

    //open and dump to file
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    if(outFile.is_open())
    {
        if(nspaces == 0)
        {
            outFile << vex;
        }
        else
        {
            outFile << vex.dump(nspaces);
        }
    }
    else
    {
        msg_error("main", "could not open file: " << output_file << eom);
    }
    outFile.close();

    msg_info("main", "done" << eom);

    return 0;
}
