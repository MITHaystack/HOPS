#include "MHO_VexBlockParser.hh"

#include <fstream>
#include <cctype>
#include <algorithm>
#include <stack>

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

mho_json
MHO_VexBlockParser::ParseBlockLines(std::string block_name, const std::vector< MHO_VexLine >* block_lines)
{
    //retrieve the block format 
    fBlockFormatLoaded = false;
    LoadBlockFormat(block_name);
    fBlockLines = block_lines;

    if(fBlockFormatLoaded){ return ParseBlock();}
    else 
    {
        mho_json empty;
        msg_error("vex", "parser error, could not load format file for: "<<block_name<<" block."<<eom);
        return empty;
    }
}

mho_json
MHO_VexBlockParser::ParseBlock()
{
    mho_json root;
    std::vector< std::string > tokens;

    std::stack< std::string > path;
    std::stack< mho_json > file_node;
    std::stack< mho_json > format_node;

    path.push( fBlockFormat["block_name"] );
    file_node.push( root[path.top()] );

    if(fBlockLines != nullptr)
    {
        for(auto it = fBlockLines->begin(); it != fBlockLines->end(); it++)
        {
            fTokenizer.SetString( &(it->fContents) );
            fTokenizer.GetTokens(&tokens);

            if(tokens.size() > 0)
            {
                if(tokens.size() >= 2 && tokens[0] == fStartTag)
                {
                    path.push( tokens[1] );
                    file_node.push( (file_node.top())[ path.top() ] );
                    format_node.push( fBlockFormat["parameters"] );
                }

                if(tokens[0] == fStopTag)
                {
                    if( !ValidateNode( file_node.top(), format_node.top() ) )
                    { 
                        std::string file_path = CollapsePath(path);
                        msg_warn("vex", "error parsing file, line: "<< it->fLineNumber<<" element: "<< file_path << eom );   
                    }
                    path.pop();
                    file_node.pop();
                    format_node.pop();
                }
            }
            
            // for(auto tmp = tokens.begin(); tmp != tokens.end(); tmp++)
            // {
            //     std::cout<<*tmp<<"|";
            // }
            // std::cout<<std::endl;
        }
    }

    return root;
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
    fBlockName = block_name;
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


std::string 
MHO_VexBlockParser::CollapsePath( std::stack< std::string >& path )
{
    std::string ret_val;
    while( path.size() != 0)
    {
        ret_val =  path.top() + "/" + ret_val;
        path.pop();
    }
    return ret_val;
}


bool 
MHO_VexBlockParser::ValidateNode( mho_json& data, mho_json& format)
{
    return false;
}

}//end namespace