#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <getopt.h>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_AFileDefinitions.hh"
#include "MHO_AFileInfoExtractor.hh"


using namespace hops;

#define DEFAULT_ALIST_VERS 6


int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();

    std::vector< std::string > input_files;
    std::string comment_char = "*";
    std::string output_file = "alist.out";
    int message_level = 5;
    int version = DEFAULT_ALIST_VERS; //default format version is alist v6 (only 5 or 6 supported)
    bool json_mode;
    
    //options of original alist program are:
    //'-o' to specify the output file name
    //'-m' to specify the message level
    //'-c' to specify the comment character
    //'-v' to specify the format version
    //the rest of the args are a positional list of files to process

    //some other arguments we want to consider implmenting:
    //(1) '-s' for sort (sort the files by time/baseline)
    //(2) '--only-<something>' to process only cor or fringe or station files, etc (moot, since we only process fringe files )
    //(3) '-j' export data as a json file

    CLI::App app{"alist"};
    app.add_option("-o,--output-file", output_file, "name of the output file (default: alist.out)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-c,--comment-character", comment_char, "the character indicating a comment line, default is '*'");
    app.add_option("-v,--version", version, "the alist version (default: 6)");
    app.add_flag("-j,--json", json_mode, "generate a json summary file instead of an 'alist' formatted file");
    app.add_option("input_files,-i,--input-files", input_files, "list of the files to process")->required();
    CLI11_PARSE(app, argc, argv);

    if(json_mode && output_file == "alist.out"){output_file = "alist.json";}

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //extract the data
    MHO_AFileInfoExtractor ext;
    std::vector<mho_json> results;
    for(std::size_t i=0; i<input_files.size(); i++)
    {
        mho_json fsum;
        bool ok = ext.SummarizeFringeFile(input_files[i], fsum);
        if(ok){results.push_back(fsum);}
    }

    //TODO -- sort the data here (if requested)
    
    std::stringstream afile_contents;
    if(!json_mode)
    {
        char com_char = comment_char[0];
        std::string afile_header = ext.GetAlistHeader(version, 2, com_char);
        afile_contents << afile_header;
        for(std::size_t i=0; i<results.size(); i++)
        {
            afile_contents << ext.ConvertToAlistRow(results[i], version);
            //if(i != input_files.size()-1 ){afile_contents << "\n";}
        }
    }
    else 
    {
        mho_json output;
        for(std::size_t i=0; i<results.size(); i++)
        {
            output["data"].push_back(results[i]);
        }
        afile_contents << output.dump(2);
    }

    //open file and dump
    std::ofstream outFile(output_file.c_str(), std::ofstream::out);
    if(outFile.is_open()){outFile << afile_contents.str();}
    outFile.close();

    return 0;
}
