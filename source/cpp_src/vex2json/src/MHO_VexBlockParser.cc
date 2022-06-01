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
    fRefTag = "ref";
    fAssignmentTag = "=";
    //fVexDelim = " :;\t\r\n";
    fVexDelim = ":;";
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
    std::stack< mho_json* > file_node;
    std::stack< mho_json > format_node;

    path.push(fBlockName);
    file_node.push( &root );

    if(fBlockLines != nullptr)
    {
        for(auto it = ++(fBlockLines->begin()); it != fBlockLines->end(); it++)
        {
            if( IsStartTag(*it) )
            {
                //open a new block
                std::cout<<"new block element with name: "<<tokens[1]<<std::endl;
                path.push( tokens[1] );
                file_node.push( new mho_json() );
                format_node.push( fBlockFormat["parameters"] );
            }
            else if( IsStopTag(*it) )
            {
                std::cout<<"closing block element: "<<path.top()<<std::endl;
                //close the existing block
                // std::string file_path = fBlockName + "/" + CollapsePath(path);
                // if( !ValidateNode( file_node.top(), format_node.top() ) )
                // { 
                //     msg_warn("vex", "could not process file element ending on line: "<< it->fLineNumber<<", path: "<< file_path << eom );   
                // }

                mho_json* last_obj = file_node.top();
                std::string last_obj_name = path.top();
                file_node.pop();
                path.pop();
                format_node.pop();
                (*(file_node.top()))[last_obj_name] = *last_obj;
                delete last_obj;
            }
            else 
            {
                //process parameters for the existing block
                bool success = ProcessTokens( file_node.top(), format_node.top(), tokens);
                if(!success){msg_warn("vex", "failed to process tokens on line: "<< it->fLineNumber << eom);}
            }
    
            // // fTokenizer.SetString( &(it->fContents) );
            // // fTokenizer.GetTokens(&tokens);
            // 
            // if(tokens.size() > 0)
            // {
            //     if(tokens.size() >= 2 && tokens[0] == fStartTag)
            //     {
            // 
            //     }
            //     else if(tokens[0] == fStopTag)
            //     {
            // 
            //     }
            //     else 
            //     {
            // 
            //     }
            // }
        }
    }

    return root;
}

bool IsStartTag(MHO_VexLine& line)
{

}

bool IsStopTag(MHO_VexLine& line)
{

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
    fStartTag = fBlockFormat["start_tag"].get<std::string>();
    fStopTag = fBlockFormat["stop_tag"].get<std::string>();

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
    while( path.size() > 1)
    {
        ret_val =  path.top() + "/" + ret_val;
        path.pop();
    }
    return ret_val;
}


bool 
MHO_VexBlockParser::ValidateNode( mho_json& data, mho_json& format)
{
    return true;
}

bool 
MHO_VexBlockParser::ProcessTokens( mho_json* data, mho_json& format, std::vector< std::string >& tokens)
{
    if(tokens[0] == fRefTag)
    {
        msg_error("vex", "error ref keyword not yet implmented." << eom);
        return false;
    }

    //check that we have an assignement statement 
    if(tokens.size() >= 2 && tokens[1] == fAssignmentTag)
    {
        std::string key = tokens[0];
        bool key_is_present = false;
        for(auto& it : format.items())
        {
            if(key == it.key()){key_is_present = true; break;}
        }

        if(key_is_present)
        {
            std::cout<<"got a key:"<<key<<std::endl;
            // //we have 3 main types to deal with (compound, list, and primitive)
            // if(format[key]["type"] == "compound" )
            // {
            //     return false;
            // }
            // else if( format[key]["type"] == "list" )
            // {
            //     return false;
            // }
            // else //primitive type (number, string, etc) 
            // {
                if( format[key]["type"] == "string" )
                {
                    (*data)[key] = tokens[2];
                    return true;
                }

                if( format[key]["type"] == "real" )
                {
                    std::cout<<"value = "<<tokens[2]<<std::endl;
                    (*data)[key] = std::atof(tokens[2].c_str());
                    return true;
                }

                if( format[key]["type"] == "int" )
                {
                    (*data)[key] = std::atoi(tokens[2].c_str());
                    return true;
                }
        //    }

            // // for(auto tmp = tokens.begin(); tmp != tokens.end(); tmp++)
            // // {
            // //     std::cout<<*tmp<<"|";
            // // }
            // // std::cout<<std::endl;
            // return true;
        }
    }

    return false;
}


}//end namespace