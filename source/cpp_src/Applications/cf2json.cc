#include <fstream>
#include <stdexcept>
#include <string>

#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlFileParser.hh"
#include "MHO_Message.hh"

using namespace hops;

#include "CLI11.hpp"

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    std::string set_string = "";
    unsigned int nspaces = 0;
    int message_level = 1;
    bool processed_text = false;
    bool legacy_text = false;

    CLI::App app{"cf2json: converts a fourfit control file to its json representation"};

    app.add_option("input,-i,--input-file", input_file, "name of the input (fourfit control) file")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output (json) file, if not given the result will be stored in <input-file>.json");
    app.add_option("-p,--pretty-print", nspaces,
                   "generate the json output with indentations (soft tabs) consisting of the number of spaces specified, "
                   "default (disabled)");
    app.add_option("-s,--set", set_string, "any additional control statements passed via 'set'");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_flag("-t,--processed-text", processed_text,
                 "instead of json, emit the processed control file text (without comments)");
    app.add_flag("-l,--legacy-text", legacy_text,
                 "instead of json, emit the legacy processed control file text (control file text, no set string)")
        ->excludes("--processed-text");

    CLI11_PARSE(app, argc, argv);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(output_file == "")
    {
        if(processed_text || legacy_text)
        {
            output_file = input_file + ".txt";
        }
        else
        {
            output_file = input_file + ".json";
        }
    }

    std::string format_dir = MHO_ControlDefinitions::GetFormatDirectory();
    std::vector< std::string > keywords = MHO_ControlDefinitions::GetKeywordNames();
    if(keywords.size() == 0)
    {
        msg_fatal("main", "no control format definitions found under: "
                              << format_dir << ", set HOPS_INSTALL to the hops install prefix or source hops.bash" << eom);
        return 2;
    }
    msg_debug("main", "using " << keywords.size() << " control keyword definitions from: " << format_dir << eom);

    mho_json root;
    MHO_ControlFileParser parser;
    parser.SetControlFile(input_file);
    if(set_string != "")
    {
        parser.PassSetString(set_string);
    }

    try
    {
        root = parser.ParseControl();
    }
    catch(const std::exception& e)
    {
        msg_fatal("main", "could not parse control file: " << input_file << ", " << e.what() << eom);
        return 1;
    }

    if(!root.contains("conditions"))
    {
        msg_fatal("main", "could not read control file: " << input_file << eom);
        return 1;
    }

    std::string text = "";
    if(processed_text)
    {
        text = parser.GetProcessedControlFileText();
    }
    else if(legacy_text)
    {
        text = parser.GetLegacyProcessedControlFileText();
    }
    else if(nspaces == 0)
    {
        text = root.dump();
    }
    else
    {
        text = root.dump(nspaces);
    }

    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    if(!outFile.is_open())
    {
        msg_fatal("main", "could not open file: " << output_file << eom);
        return 1;
    }
    outFile << text << std::endl;
    outFile.close();

    msg_info("main", "wrote " << output_file << eom);

    return 0;
}
