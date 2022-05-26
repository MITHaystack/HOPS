#include "MHO_VexBlockParser.hh"

namespace hops 
{

MHO_VexBlockParser::MHO_VexBlockParser()
{
    fBlockName = "";
    fBlockLines = nullptr;
    fStartTag = "def";
    fStopTag = "enddef";
};

MHO_VexBlockParser::~MHO_VexBlockParser(){};

void
MHO_VexBlockParser::SetBlockFormat(mho_json block_def);
{
    fBlockFormat = block_def;
    fBlockName = fBlockFormat["block_name"].get<std::string>();
}

void 
MHO_VexBlockParser::SetBlockLines(std::string block_name, const std::vector< std::string >* block_lines)
{
    fBlockLines = nullptr;
    if(fBlockName == block_name)
    {
        fBlockLines = block_lines;
    }
    else 
    {
        msg_error("vex", "parser error, block name: " <<block_name<<" does not match currently set block format: "<<fBlockName<<eom);
    }
}

void 
MHO_VexBlockParser::ParseBlock()
{
    if(fBlockLines != nullptr)
    {
        
    
    }
}



}