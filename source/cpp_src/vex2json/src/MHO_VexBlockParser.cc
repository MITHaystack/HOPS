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
    fVexDelim = " :;\t\r\n";
    fTokenizer.SetDelimiter(fVexDelim);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetPreserveQuotesTrue();
};

MHO_VexBlockParser::~MHO_VexBlockParser(){};

void 
MHO_VexBlockParser::ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines)
{
    //retrieve the block format 
    fBlockFormatLoaded = false;
    LoadBlockFormat(block_name);
    fBlockLines = block_lines;

    if(fBlockFormatLoaded)
    {
        //parses the block according to the format rules
        ParseBlock();
    }
    else 
    {
        msg_error("vex", "parser error, could not load format file for: "<<block_name<<" block."<<eom);
    }
}

void 
MHO_VexBlockParser::ParseBlock()
{
    mho_json block; //place to stash the data 
    std::vector< std::string > tokens;
    if(fBlockLines != nullptr)
    {
        for(auto it = fBlockLines->begin(); it != fBlockLines->end(); it++)
        {
            fTokenizer.SetString( &(it->fContents) );
            fTokenizer.GetTokens(&tokens);

            for(auto tmp = tokens.begin(); tmp != tokens.end(); tmp++)
            {
                std::cout<<*tmp<<"|";
            }
            std::cout<<std::endl;
        }
    }
}

void 
MHO_VexBlockParser::LoadBlockFormat(std::string block_name)
{
    fBlockFormatLoaded = false;
    std::string block_format_file = GetBlockFormatFileName(block_name);
    std::string format_file = fFormatDirectory + block_format_file;

    std::cout<<"block_format file = "<<format_file<<std::endl;

    std::ifstream bf_ifs;
    bf_ifs.open( format_file.c_str(), std::ifstream::in );

    mho_json bformat;
    if(bf_ifs.is_open())
    {
        bformat = mho_json::parse(bf_ifs);
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