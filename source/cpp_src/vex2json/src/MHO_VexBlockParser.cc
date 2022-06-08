#include "MHO_VexBlockParser.hh"

#include <fstream>
#include <cctype>
#include <algorithm>
#include <stack>
#include <regex>

namespace hops 
{

MHO_VexBlockParser::MHO_VexBlockParser()
{
    fBlockLines = nullptr;
    fStartTag = "def";
    fStopTag = "enddef";
    fChanDefTag = "chan_def";
    fIFDefTag = "if_def";
    fRefTag = "ref";
    fAssignmentDelim = "=;";
    //fVexDelim = " :;\t\r\n";
    fWhitespaceDelim = " \t\r\n";
    fVexDelim = ":";
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
    std::vector< std::string > tokens;
    fTokenizer.SetDelimiter(fStartTagDelim);
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetIncludeEmptyTokensFalse();
    fTokenizer.SetPreserveQuotesFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() > 1)
    {
        if(tokens[0] == fStartTag){return true;}
    }
    return false;

    // if(line.fContents.find(fStopTag) != std::string::npos){return false;}
    // if(line.fContents.find(fChanDefTag) != std::string::npos){return false;}
    // if(line.fContents.find(fIFDefTag) != std::string::npos){return false;}
    // if(line.fContents.find(fStartTag)!= std::string::npos){return true;}
    // return false;
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
    fTokenizer.SetIncludeEmptyTokensFalse();

    std::vector< std::string > tokens;
    fTokenizer.SetString(&(line.fContents));
    fTokenizer.GetTokens(&tokens);

    //verify that n_tokens is 2 for '=' assignment statement
    if(tokens.size() == 2)
    {
        std::string element_name = tokens[0];
        std::cout<<"element  name = "<<tokens[0]<<std::endl;
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
        std::string data = tokens[1]; //everything between '=' and ";"
        fTokenizer.SetDelimiter(fVexDelim);
        fTokenizer.SetIncludeEmptyTokensTrue();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetString(&data);
        fTokenizer.GetTokens(&tokens);

        std::cout<<"**** tokens **** "<<std::endl;
        for(auto it = tokens.begin(); it != tokens.end(); it++)
        {
            std::cout<<"| "<< *it << "|";
        }
        std::cout<<std::endl;

        vex_element_type etype = DetermineType( format[element_name]["type"].get<std::string>() );
        mho_json element = ProcessTokens(element_name, format[element_name], tokens);

        if( etype == vex_list_compound_type)
        {
            (*obj_node)[element_name].push_back( element ); 
        }
        else 
        {
            (*obj_node)[element_name] = element; 
        }
        return true;
    }
    else 
    {
        msg_error("vex", "expected assignment with 2 tokens, but could not determine how to parse "<<tokens.size()<<" tokens from: <"<<line.fContents<<">."<<eom);
        return false;
    }

}

mho_json 
MHO_VexBlockParser::ProcessTokens(const std::string& element_name, mho_json& format, std::vector< std::string >& tokens)
{
    vex_element_type etype = DetermineType( format["type"].get<std::string>() );
    mho_json element_data;

    switch(etype)
    {
        case vex_int_type:
            element_data = ProcessInt(element_name, format, tokens);
        break;
        case vex_list_int_type:
            element_data = ProcessListInt(element_name, format, tokens);
        break;
        case vex_real_type:
            element_data = ProcessReal(element_name, format, tokens);
        break;
        case vex_list_real_type:
            element_data = ProcessListReal(element_name, format, tokens);
        break;
        case vex_string_type:
            std::cout<<"processing a string: "<<tokens[0]<<std::endl;
            element_data = tokens[0];
        break;
        case vex_list_string_type:
            element_data = ProcessListString(element_name, format, tokens);
        break;
        case vex_epoch_type:
            element_data = tokens[0];
        break;
        case vex_link_type:
            element_data = tokens[0];
        break;
        case vex_compound_type: //all compound types treated the same way
            element_data = ProcessCompound(element_name, format, tokens);
        case vex_list_compound_type:
            element_data = ProcessCompound(element_name, format, tokens);
        break;
        default:
        break;
    }
    return element_data;
}

mho_json 
MHO_VexBlockParser::ProcessInt(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data = std::atoi(tokens[0].c_str());
    return element_data;
}

mho_json 
MHO_VexBlockParser::ProcessListInt(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< int > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        values.push_back( std::atof(tokens[i].c_str() ) );
    }
    element_data = values;
    return element_data;
}


mho_json 
MHO_VexBlockParser::ProcessListString(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< std::string > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        values.push_back( tokens[i] );
    }
    element_data = values;
    return element_data;
}


mho_json 
MHO_VexBlockParser::ProcessReal(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data;

    std::cout<<"real value = "<<tokens[0]<<std::endl;
    //std::cout<<"element_name     = "<<element_name<<std::endl;

    if( ContainsWhitespace(tokens[0]) )  //if the value has units (we need to parse them out)
    {
        std::vector< std::string > tmp_tok;
        fTokenizer.SetString(&(tokens[0]));
        fTokenizer.SetDelimiter(fWhitespaceDelim);
        fTokenizer.SetUseMulticharacterDelimiterFalse();
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.GetTokens(&tmp_tok);
        if(tmp_tok.size() == 1)
        {
            element_data["value"] = std::atof(tmp_tok[0].c_str());
        }
        else if(tmp_tok.size() == 2)
        {
            element_data["value"] = std::atof(tmp_tok[0].c_str());
            element_data["units"] = tmp_tok[1];
        }
        else 
        {
            msg_error("vex", "could not parse parameter: "<<element_name<<", as (numerical) value from: <"<<tokens[0]<<">."<<eom);
        }
    }
    else 
    {
        element_data["value"] = std::atof(tokens[0].c_str());
    }
    return element_data;
}

mho_json 
MHO_VexBlockParser::ProcessListReal(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data;
    std::vector< double > values; 
    for(std::size_t i=0; i<tokens.size(); i++)
    {
        if( ContainsWhitespace(tokens[i]) )  //if the value has units (we need to parse them out)
        {
            std::vector< std::string > tmp_tok;
            fTokenizer.SetString(&(tokens[i]));
            fTokenizer.SetDelimiter(fWhitespaceDelim);
            fTokenizer.SetUseMulticharacterDelimiterFalse();
            fTokenizer.SetIncludeEmptyTokensFalse();
            fTokenizer.GetTokens(&tmp_tok);
            if(tmp_tok.size() == 1)
            {
                values.push_back( std::atof(tokens[i].c_str() ) );
            }
            else if(tmp_tok.size() == 2)
            {
                values.push_back( std::atof(tokens[i].c_str() ) );
                element_data["units"] = tmp_tok[1];
            }
            else 
            {
                msg_error("vex", "could not parse parameter: "<<element_name<<", as (numerical) value from: <"<<tokens[i]<<">."<<eom);
            }
        }
        else 
        {
            values.push_back( std::atof(tokens[i].c_str() ) );
        }
    }
    element_data["values"] = values;
    return element_data;
}

mho_json 
MHO_VexBlockParser::ProcessCompound(const std::string& element_name, mho_json&format, std::vector< std::string >& tokens)
{
    mho_json element_data;
    mho_json fields = format["fields"]; 
    std::size_t n_tokens = tokens.size();
    std::size_t n_all_fields = fields.size();

    for(auto it = tokens.begin(); it != tokens.end(); it++)
    {
        std::cout<< *it <<", ";
    }
    std::cout<<std::endl;

    std::size_t token_idx = 0;
    std::string hash = "#";
    std::string nothing = "";
    for(auto it = fields.begin(); it != fields.end(); it++)
    {
        if(token_idx < tokens.size() )
        {
            if(tokens[token_idx] == nothing){token_idx++;} //empty value, skip this element
            else 
            {
                std::string tmp = it->get<std::string>();
                std::string field_name = std::regex_replace(tmp,std::regex(hash),nothing);
                mho_json next_format =  format["parameters"][field_name];
                std::cout<<"field_name = "<<field_name<<std::endl;
                std::cout<<" --- --- "<<tokens[token_idx]<<std::endl;
                std::string type_name = next_format["type"].get<std::string>();

                std::vector< std::string > tmp_tokens;


                if( type_name == "list_int" || type_name == "list_real" || type_name == "list_string")
                {
                    //consume the rest of the tokens until the end
                    for(std::size_t i = token_idx; i<tokens.size(); i++){tmp_tokens.push_back(tokens[i]);};
                    element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
                    token_idx = tokens.size();
                }
                else if( MatchesType( tokens[token_idx], type_name ) )
                {
                    tmp_tokens.push_back(tokens[token_idx]);
                    element_data[field_name] = ProcessTokens(field_name, next_format, tmp_tokens);
                    token_idx++;
                }
                else 
                {
                    //assume that if the current token does not match the current type,
                    //then an optional element has been omitted in the vex file
                    //so don't increment the token index, and see if we can process it as 
                    //the next expected field
                    msg_debug("vex", "could not parse <"<<tokens[token_idx]<<">  as type: "<<type_name<<" for field: "<<field_name<< eom);
                }
            }
        }
    }
    return element_data;
}



MHO_VexBlockParser::vex_element_type 
MHO_VexBlockParser::DetermineType(std::string etype)
{
    if(etype == "int"){return vex_int_type;}
    if(etype == "list_int"){return vex_list_int_type;}
    if(etype == "real"){return vex_real_type;}
    if(etype == "list_real"){return vex_list_real_type;}
    if(etype == "epoch"){return vex_epoch_type;}
    if(etype == "string"){return vex_string_type;}
    if(etype == "list_string"){return vex_list_string_type;}
    if(etype == "compound"){return vex_compound_type;}
    if(etype == "list_compound"){return vex_list_compound_type;}
    if(etype == "link"){return vex_link_type;}
    return vex_unknown_type;
}

bool 
MHO_VexBlockParser::MatchesType(const std::string& token, const std::string& type_name)
{
    vex_element_type etype = DetermineType(type_name);
    switch(etype)
    {
        case vex_int_type:
            {
                std::string tmp = token; //needed for negative and explicitly positive integers
                if(token[0] == '-' || token[0] == '+'){tmp = token.substr(1);}
                if(tmp.find_first_not_of("0123456789") == std::string::npos)
                {
                    return true;
                }
                return false;
            }
        break;
        case vex_real_type:
            {
                std::string tmp = token;
                std::vector< std::string > tmp_tok;
                if( ContainsWhitespace(token) )  //if the value has units (we need to parse them out)
                {

                    fTokenizer.SetString(&token);
                    fTokenizer.SetDelimiter(fWhitespaceDelim);
                    fTokenizer.SetUseMulticharacterDelimiterFalse();
                    fTokenizer.SetIncludeEmptyTokensFalse();
                    fTokenizer.GetTokens(&tmp_tok);
                    if(tmp_tok.size() == 1 || tmp_tok.size() == 2){tmp = tmp_tok[0];}
                }
                if(tmp.find_first_not_of("0123456789.e+-") == std::string::npos)
                {
                    return true;
                }
                return false;
            }
        break;
        case vex_list_real_type:
            return true;//check for float list with or without units
        break;
        case vex_string_type:
            return true; //always convertable to a string
        break;
        case vex_epoch_type:
            //any epoch we encounter must at the very least include a 'year'
            //TODO FIXME -- should we make this more strict and check for day/hour etc?
            if(token.find_first_of("y") == std::string::npos)
            {
                return false;
            }
            return true;
        break;
        case vex_link_type:
            if(token.find_first_of("&") == std::string::npos){return false;}
            else{return true;}
        break;
        case vex_compound_type: 
            return true; //return true until we recurse to a simpler type
        case vex_list_compound_type:
            return true; //return true until we recurse to a simpler type
        break;
        default:
        break;
    }
    return false;
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
    auto start = value.find_first_of(fWhitespaceDelim);
    if(start == std::string::npos){return false;}
    return true;
}

}//end namespace