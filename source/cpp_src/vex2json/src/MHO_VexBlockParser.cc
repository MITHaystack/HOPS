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
    fAssignmentDelim = "=";
    //fVexDelim = " :;\t\r\n";
    fWhitespaceDelim = " \t\r\n";
    fVexDelim = ":;";
    fStartTagDelim = fWhitespaceDelim + ";";
    fTokenizer.SetDelimiter(fVexDelim);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceFalse();
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

    std::stack< std::string > path;
    std::stack< mho_json* > file_node;
    std::stack< mho_json > format_node;

    path.push(fBlockName);
    file_node.push( &root );

    if(fBlockLines != nullptr)
    {
        for(auto it = ++(fBlockLines->begin()); it != fBlockLines->end(); it++)
        {
            bool success = false;
            if( IsStartTag(*it) )
            {
                success = ProcessStartTag(*it, path, file_node, format_node);
            }
            else if( IsStopTag(*it) )
            {
                success = ProcessStopTag(*it, path, file_node, format_node);
            }
            else 
            {
                success = ProcessLine(*it, path, file_node.top(), format_node.top());
            }
            if(!success){msg_error("vex", "failed to process line: "<< it->fLineNumber << eom);}
        }
    }
    else
    {
        msg_error("vex", "failed to parse block, no lines to process."<< eom);
    }

    return root;
}

bool
MHO_VexBlockParser::IsStartTag(const MHO_VexLine& line)
{
    if(line.fContents.find(fStopTag) != std::string::npos){return false;}
    if(line.fContents.find(fStartTag)!= std::string::npos){return true;}
    return false;
}

bool 
MHO_VexBlockParser::IsStopTag(const MHO_VexLine& line)
{
    if(line.fContents.find(fStopTag) != std::string::npos){return true;}
    return false;
}

bool 
MHO_VexBlockParser::ProcessStartTag(const MHO_VexLine& line, 
                     std::stack< std::string >& path,
                     std::stack< mho_json* >& file_node,
                     std::stack< mho_json >& format_node)
{
    fTokenizer.SetDelimiter(fStartTagDelim);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetPreserveQuotesFalse();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);

    if(tokens.size() < 2)
    {
        msg_error("vex", "error processing block start tag, too few tokens."<<eom);
        return false;
    }

    //open a new block
    msg_debug("vex", "opening a new block element with name: "<<tokens[1]<<eom);
    path.push( tokens[1] );
    file_node.push( new mho_json() );
    format_node.push( fBlockFormat["parameters"] ); //TODO FIXME -- check that 'parameters' exists!
    return true;
}

bool 
MHO_VexBlockParser::ProcessStopTag(const MHO_VexLine& line, 
                      std::stack< std::string >& path,
                      std::stack< mho_json* >& file_node,
                      std::stack< mho_json >& format_node)
{
    //close current block
    mho_json* last_obj = file_node.top();
    std::string last_obj_name = path.top();
    file_node.pop();
    path.pop();
    format_node.pop();
    //insert this object into output json structure
    (*(file_node.top()))[last_obj_name] = *last_obj; 
    delete last_obj;
    msg_debug("vex", "closed a block element with name: "<<last_obj_name<<eom);
    return true;
}

bool 
MHO_VexBlockParser::ProcessLine(const MHO_VexLine& line, 
                 std::stack< std::string >& path,
                 mho_json* obj_node,
                 mho_json& format)
{
    fTokenizer.SetDelimiter(fAssignmentDelim);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensTrue();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);

    //verify that n_tokens is 2 
    if(tokens.size() == 2)
    {
        std::string element_name = tokens[0];

        //verify that the element name is present in the current format node 
        bool key_is_present = false;
        for(auto& it : format.items()){if(element_name == it.key()){key_is_present = true; break;}}
        if(!key_is_present)
        {
            msg_error("vex", "could not locate element with name: "<<element_name<<" under current format block."<<eom);
            return false;
        }

        //for very specific lines (i.e. source coordinates, we may need to set this to false)
        fTokenizer.SetPreserveQuotesTrue();

        //for very specific lines (i.e. source coordinates, we may need to set this to false)
        //fTokenizer.SetPreserveQuotesFalse();

        //data exists to the right of '=', before the ';'
        std::string data = tokens[1]; //statements to the right of the assignment '='
        fTokenizer.SetDelimiter(fVexDelim);
        fTokenizer.SetString(&data);
        fTokenizer.GetTokens(&tokens);

        mho_json element = ProcessTokens(format[element_name], tokens);
        (*obj_node)[element_name] = element; 
        return true;
    }
    else 
    {
        return false;
        msg_error("vex", "expected assignment, but could not determine how to parse."<<eom);
    }

}

mho_json 
MHO_VexBlockParser::ProcessTokens(mho_json& format, std::vector< std::string >& tokens)
{
    vex_element_type etype = DetermineType( format["type"].get<std::string>() );
    mho_json element_data;

    switch(etype)
    {
        case vex_int_type:
            element_data["value"] = std::atoi(tokens[0].c_str());
        break;
        case vex_real_type:
            //if the value has units (we need to parse them out)
            if( ContainsWhitespace(tokens[0]) )
            {
                std::vector< std::string > tmp_tok;
                fTokenizer.SetString(&(tokens[0]));
                fTokenizer.SetDelimiter(fWhitespaceDelim);
                fTokenizer.SetUseMulticharacterDelimiterFalse();
                fTokenizer.SetIncludeEmptyTokensFalse();
                fTokenizer.GetTokens(&tmp_tok);
                if(tmp_tok.size() == 1)
                {
                    element_data["value"] = std::atof(tokens[0].c_str());
                }
                else if(tmp_tok.size() == 2)
                {
                    element_data["value"] = std::atof(tmp_tok[0].c_str());
                    element_data["units"] = tmp_tok[1];
                }
                else 
                {
                    msg_error("vex", "could not parse parameter (numerical) value from: {"<<tokens[0]<<"}."<<eom);
                }
            }
            else 
            {
                element_data["value"] = std::atof(tokens[0].c_str());
            }
        break;
        case vex_string_type:
            element_data["value"] = tokens[0];
        break;
        case vex_link_type:
            element_data["value"] = tokens[0];
        break;
        case vex_compound_type:
            mho_json fields = format["fields"];

            std::size_t n_tokens = tokens.size()
            std::size_t n_fields = fields.size();
            
            for(auto it = format["fields"].begin(); it != format 


        break;
        default:
        break;
    }

    return element_data;
}


MHO_VexBlockParser::vex_element_type 
MHO_VexBlockParser::DetermineType(std::string etype)
{
    if(etype == "int"){return vex_int_type;}
    if(etype == "real"){return vex_real_type;}
    if(etype == "string"){return vex_string_type;}
    if(etype == "compound"){return vex_compound_type;}
    if(etype == "link"){return vex_link_type;}
    return vex_unknown_type;
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

    if(fBlockFormatLoaded)
    {
        fBlockFormat = bformat;
        fBlockName = block_name;
        fStartTag = fBlockFormat["start_tag"].get<std::string>();
        fStopTag = fBlockFormat["stop_tag"].get<std::string>();
    }
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

bool 
MHO_VexBlockParser::ContainsWhitespace(std::string value)
{
    auto start = value.find_first_not_of(fWhitespaceDelim);
    if(start == std::string::npos){return false;}
    return true;
}


}//end namespace