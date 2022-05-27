#include "MHO_VexBlockParser.hh"

#include <fstream>
#include <cctype>
#include <algorithm>

namespace hops 
{

MHO_VexBlockParser::MHO_VexBlockParser()
{
    fBlockLines = nullptr;
    fStartTag = "def";
    fStopTag = "enddef";
};

MHO_VexBlockParser::~MHO_VexBlockParser(){};

void 
MHO_VexBlockParser::SetBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines)
{
    //retrieve the block format 
    LoadBlockFormat(block_name);
    fBlockLines = block_lines;

    if(fBlockFormatLoaded)
    {
        //pares the block according to the format rules
    }
    else 
    {
        msg_error("vex", "parser error, could not load format file for: "<<block_name<<" block."<<eom);
    }

    // fBlockLines = nullptr;
    // if(fBlockName == block_name)
    // {
    //     fBlockLines = block_lines;
    // }
    // else 
    // {
    //     msg_error("vex", "parser error, block name: " <<block_name<<" does not match currently set block format: "<<fBlockName<<eom);
    // }
}

void 
MHO_VexBlockParser::ParseBlock()
{
    if(fBlockLines != nullptr)
    {
        
    
    }
}

void 
MHO_VexBlockParser::LoadBlockFormat(std::string block_name)
{
    std::string block_format_file = GetBlockFormatFileName(block_name);
    std::string format_file = fFormatDirectory + block_format_file;

    std::cout<<"block_format file = "<<format_file<<std::endl;

    std::ifstream bf_ifs;
    bf_ifs.open( format_file.c_str(), std::ifstream::in );
    
    fBlockFormatLoaded = false;
    mho_json bformat;
    if(bf_ifs.is_open())
    {
        bformat = mho_ordered_json::parse(bf_ifs);
        std::cout<< bformat.dump(2) << std::endl;
        fBlockFormatLoaded = true;
    }
    bf_ifs.close();

    fBlockFormat = bformat;
}

std::string 
MHO_VexBlockParser::GetBlockFormatFileName(std::string block_name)
{
    //remove '$', and convert to lower-case
    std::string file_name = block_name.substr(1);
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    file_name += ".json";
    return file_name;
}



}